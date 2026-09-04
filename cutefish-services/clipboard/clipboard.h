#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QClipboard>

class WlrClipboard;

class Clipboard : public QObject
{
    Q_OBJECT

public:
    explicit Clipboard(QObject *parent = nullptr);

private slots:
    void onDataChanged();

private:
    QClipboard *m_qtClipboard = nullptr;
    WlrClipboard *m_waylandClipboard = nullptr;
};

#endif // CLIPBOARD_H
