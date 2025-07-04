#include "codeeditorwindow.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QProcess>
#include <QApplication>

CodeEditorWindow::CodeEditorWindow(QWidget *parent)
    : QMainWindow(parent), currentFile(""), currentFolder("") {
    editor = new QTextEdit(this);
    setCentralWidget(editor);

    highlighter = new SyntaxHighlighter(editor->document());

    keyPressHandler = new KeyPressHandler(editor, this);
    connect(keyPressHandler, &KeyPressHandler::saveRequested, this, &CodeEditorWindow::saveFile);
    connect(keyPressHandler, &KeyPressHandler::undoRequested, editor, &QTextEdit::undo);

    QMenu *fileMenu = menuBar()->addMenu("Файл");
    QAction *newFile = fileMenu->addAction("Новый файл");
    QAction *openFile = fileMenu->addAction("Открыть файл");
    QAction *openFolder = fileMenu->addAction("Открыть папку");
    QAction *saveFile = fileMenu->addAction("Сохранить файл");

    connect(newFile, &QAction::triggered, this, &CodeEditorWindow::createNewFile);
    connect(openFile, &QAction::triggered, this, &CodeEditorWindow::openFile);
    connect(openFolder, &QAction::triggered, this, &CodeEditorWindow::openFolder);
    connect(saveFile, &QAction::triggered, this, &CodeEditorWindow::saveFile);

    QMenu *viewMenu = menuBar()->addMenu("Вид");
    QAction *darkTheme = viewMenu->addAction("Тёмная тема");
    QAction *lightTheme = viewMenu->addAction("Светлая тема");

    connect(darkTheme, &QAction::triggered, this, &CodeEditorWindow::switchToDarkTheme);
    connect(lightTheme, &QAction::triggered, this, &CodeEditorWindow::switchToLightTheme);

    fileTreeDock = new QDockWidget("Файлы", this);
    fileTree = new QTreeView(fileTreeDock);
    fileModel = new QFileSystemModel(this);
    fileTree->setModel(fileModel);
    fileTreeDock->setWidget(fileTree);
    addDockWidget(Qt::LeftDockWidgetArea, fileTreeDock);

    connect(fileTree, &QTreeView::doubleClicked, this, &CodeEditorWindow::openFileFromTree);

    terminal = new QPlainTextEdit(this);
    terminal->setReadOnly(true);
    QDockWidget *terminalDock = new QDockWidget("Терминал", this);
    terminalDock->setWidget(terminal);
    addDockWidget(Qt::BottomDockWidgetArea, terminalDock);

    QToolBar *toolBar = addToolBar("Инструменты");
    QAction *buildAction = toolBar->addAction("Собрать");
    QAction *runAction = toolBar->addAction("Запустить");

    connect(buildAction, &QAction::triggered, this, &CodeEditorWindow::buildProject);
    connect(runAction, &QAction::triggered, this, &CodeEditorWindow::runProject);

    loadLastFolder();
}

void CodeEditorWindow::applyDarkTheme() {
    qApp->setStyleSheet("QTextEdit { background-color: #2b2b2b; color: #ffffff; }"
                        "QTreeView { background-color: #2b2b2b; color: #ffffff; }"
                        "QPlainTextEdit { background-color: #2b2b2b; color: #ffffff; }");
}

void CodeEditorWindow::applyLightTheme() {
    qApp->setStyleSheet("");
}

void CodeEditorWindow::buildProject() {
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

void CodeEditorWindow::runProject() {
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
#include "main.moc"