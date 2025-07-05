#include "keypresshandler.h"

KeyPressHandler::KeyPressHandler(QTextEdit *editor, QObject *parent)
    : QObject(parent), editor(editor)
{
    editor->installEventFilter(this);
}

bool KeyPressHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == editor && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::ControlModifier)
        {
            if (keyEvent->key() == Qt::Key_S)
            {
                emit saveRequested();
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Z)
            {
                emit undoRequested();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}