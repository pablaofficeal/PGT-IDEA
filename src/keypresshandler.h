#ifndef KEYPRESSHANDLER_H
#define KEYPRESSHANDLER_H

#include <QObject>
#include <QKeyEvent>
#include <QTextEdit>

class KeyPressHandler : public QObject {
    Q_OBJECT

public:
    explicit KeyPressHandler(QTextEdit *editor, QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void saveRequested();
    void undoRequested();

private:
    QTextEdit *editor;
};

#endif // KEYPRESSHANDLER_H