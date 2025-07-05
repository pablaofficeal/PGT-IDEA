#ifndef AFTOCOMPLET_H
#define AFTOCOMPLET_H

#include <QString>
#include <QList>
#include <QTextEdit>

class aftocomplet {
public:
    explicit aftocomplet(QTextEdit *editor, QObject *parent = nullptr);
    QList<QString> suggestCompletions(const QString &text);
    void showCompletionPopup(const QString &text);

private:
    QTextEdit *editor;
    QStringList keywords;
};

#endif // AFTOCOMPLET_H