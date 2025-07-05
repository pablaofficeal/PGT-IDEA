#ifndef CODEEDITORWINDOW_H
#define CODEEDITORWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPlainTextEdit>
#include <QDockWidget>
#include <QToolBar>
#include "syntaxhighlighter.h"
#include "keypresshandler.h"

class CodeEditorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit CodeEditorWindow(QWidget *parent = nullptr);

private slots:
    void createNewFile();
    void openFile();
    void saveFile();
    void openFolder();
    void loadLastFolder();
    void openFileFromTree(const QModelIndex &index);
    void buildProject();
    void runProject();
    void switchToDarkTheme();
    void switchToLightTheme();

private:
    void loadFile(const QString &fileName);
    void applyDarkTheme();
    void applyLightTheme();

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

#endif // CODEEDITORWINDOW_H