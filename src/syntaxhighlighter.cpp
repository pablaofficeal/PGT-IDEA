#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywords = {
        "int", "float", "if", "else", "while", "return", "class",
        "import", "start", "Class", "print", "give", "function",
        "Method", "connect", "from", "Path", "OSS", "protocol", "ip",
        "port", "http", "https", "DNS", "ip-v4", "ip-v6", "break",
        "open", "close", "mkadir", "rmdir", "delete", "rename",
        "void", "stdout", "stderr", "list", "lib", "create",
        "std", "vg", "trs", "exit", "return", "else if"};

    for (const QString &keyword : keywords)
    {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + keyword + "\\b");
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::darkGreen);
    HighlightingRule commentRule;
    commentRule.pattern = QRegularExpression("//[^\n]*");
    commentRule.format = commentFormat;
    highlightingRules.append(commentRule);

    QTextCharFormat stringFormat;
    stringFormat.setForeground(Qt::darkRed);
    HighlightingRule stringRule;
    stringRule.pattern = QRegularExpression("\".*\"");
    stringRule.format = stringFormat;
    highlightingRules.append(stringRule);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules)
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}