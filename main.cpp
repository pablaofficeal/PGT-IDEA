#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDockWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QSettings>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QProcess>
#include <QSocketNotifier>
#include <iostream>
#include "syntaxhighlighter.h"
#include "keypresshandler.h"

class CodeEditor : public QMainWindow {
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr) : QMainWindow(parent) {
        editor = new QTextEdit(this);
        setCentralWidget(editor);

        highlighter = new SyntaxHighlighter(editor->document());

        keyPressHandler = new KeyPressHandler(editor, this);
        connect(keyPressHandler, &KeyPressHandler::saveRequested, this, &CodeEditor::saveFile);
        connect(keyPressHandler, &KeyPressHandler::undoRequested, editor, &QTextEdit::undo);

        // Создание меню
        QMenu *fileMenu = menuBar()->addMenu("Файл");
        QAction *newFile = fileMenu->addAction("Новый файл");
        QAction *openFile = fileMenu->addAction("Открыть файл");
        QAction *openFolder = fileMenu->addAction("Открыть папку");
        QAction *saveFile = fileMenu->addAction("Сохранить файл");

        connect(newFile, &QAction::triggered, this, &CodeEditor::createNewFile);
        connect(openFile, &QAction::triggered, this, &CodeEditor::openFile);
        connect(openFolder, &QAction::triggered, this, &CodeEditor::openFolder);
        connect(saveFile, &QAction::triggered, this, &CodeEditor::saveFile);

        // Боковая панель с деревом файлов
        fileTreeDock = new QDockWidget("Файлы", this);
        fileTree = new QTreeView(fileTreeDock);
        fileModel = new QFileSystemModel(this);
        fileTree->setModel(fileModel);
        fileTreeDock->setWidget(fileTree);
        addDockWidget(Qt::LeftDockWidgetArea, fileTreeDock);

        connect(fileTree, &QTreeView::doubleClicked, this, &CodeEditor::openFileFromTree);

        // Терминал
        terminal = new QPlainTextEdit(this);
        terminal->setReadOnly(true);
        QDockWidget *terminalDock = new QDockWidget("Терминал", this);
        terminalDock->setWidget(terminal);
        addDockWidget(Qt::BottomDockWidgetArea, terminalDock);

        // Панель инструментов
        QToolBar *toolBar = addToolBar("Инструменты");
        QAction *buildAction = toolBar->addAction("Собрать");
        QAction *runAction = toolBar->addAction("Запустить");

        connect(buildAction, &QAction::triggered, this, &CodeEditor::buildProject);
        connect(runAction, &QAction::triggered, this, &CodeEditor::runProject);

        // Загрузка последней папки
        loadLastFolder();

        // Асинхронное чтение из stdin
        QSocketNotifier *notifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, &CodeEditor::processConsoleInput);
    }

private slots:
    void processConsoleInput() {
        QTextStream input(stdin);
        QString command = input.readLine().trimmed();

        if (command == "help") {
            std::cout << "Доступные команды:\n";
            std::cout << "  help - показать список команд\n";
            std::cout << "  start theme dark - включить тёмную тему\n";
            std::cout << "  start theme light - включить светлую тему\n";
        } else if (command == "start theme dark") {
            applyDarkTheme();
        } else if (command == "start theme light") {
            applyLightTheme();
        } else {
            std::cout << "Неизвестная команда. Введите 'help' для списка команд.\n";
        }
    }

    void createNewFile() {
        editor->clear();
        currentFile.clear();
    }

    void openFile() {
        QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Все файлы (*.*)");
        if (fileName.isEmpty()) return;

        loadFile(fileName);
    }

    void saveFile() {
        QString fileName = currentFile.isEmpty()
                               ? QFileDialog::getSaveFileName(this, "Сохранить файл", "", "Все файлы (*.*)")
                               : currentFile;
        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл.");
            return;
        }

        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();
        currentFile = fileName;
    }

    void openFolder() {
        QString dir = QFileDialog::getExistingDirectory(this, "Открыть папку");
        if (dir.isEmpty()) return;

        QSettings settings("PablaIDE", "CodeEditor");
        settings.setValue("lastFolderPath", dir);

        fileModel->setRootPath(dir);
        fileTree->setRootIndex(fileModel->index(dir));
    }

    void loadLastFolder() {
        QSettings settings("PablaIDE", "CodeEditor");
        QString lastPath = settings.value("lastFolderPath", "").toString();
        if (!lastPath.isEmpty() && QDir(lastPath).exists()) {
            fileModel->setRootPath(lastPath);
            fileTree->setRootIndex(fileModel->index(lastPath));
        }
    }

    void openFileFromTree(const QModelIndex &index) {
        QFileInfo fileInfo = fileModel->fileInfo(index);
        if (fileInfo.isFile()) {
            loadFile(fileInfo.filePath());
        }
    }

    void loadFile(const QString &fileName) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
            return;
        }

        QTextStream in(&file);
        editor->setText(in.readAll());
        file.close();
        currentFile = fileName;
    }

    void buildProject() {
        if (currentFolder.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Сначала откройте папку проекта.");
            return;
        }

        QProcess *process = new QProcess(this);
        connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
            terminal->appendPlainText(process->readAllStandardOutput());
        });
        connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
            terminal->appendPlainText(process->readAllStandardError());
        });

        terminal->appendPlainText("Сборка проекта...");
        process->setWorkingDirectory(currentFolder);
        process->start("cmake --build .");
    }

    void runProject() {
        if (currentFolder.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Сначала откройте папку проекта.");
            return;
        }

        QProcess *process = new QProcess(this);
        connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
            terminal->appendPlainText(process->readAllStandardOutput());
        });
        connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
            terminal->appendPlainText(process->readAllStandardError());
        });

        terminal->appendPlainText("Запуск проекта...");
        process->setWorkingDirectory(currentFolder);
        process->start("./untitled20");
    }

    void applyDarkTheme() {
        qApp->setStyleSheet("QTextEdit { background-color: #2b2b2b; color: #ffffff; }"
                            "QTreeView { background-color: #2b2b2b; color: #ffffff; }"
                            "QPlainTextEdit { background-color: #2b2b2b; color: #ffffff; }");
        std::cout << "Тёмная тема активирована.\n";
    }

    void applyLightTheme() {
        qApp->setStyleSheet("");
        std::cout << "Светлая тема активирована.\n";
    }

private:
    QTextEdit *editor;
    SyntaxHighlighter *highlighter;
    KeyPressHandler *keyPressHandler;
    QDockWidget *fileTreeDock;
    QTreeView *fileTree;
    QFileSystemModel *fileModel;
    QPlainTextEdit *terminal;
    QString currentFile;
    QString currentFolder;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CodeEditor editor;
    editor.setWindowTitle("Pabla IDE");
    editor.resize(1000, 600);
    editor.show();
    return app.exec();
}

#include "main.moc"