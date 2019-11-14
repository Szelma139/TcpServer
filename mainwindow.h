#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDataStream>
#include <QTextStream>
#include <QTcpServer>
#include <QHash>
#include <QByteArray>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_disconnectClients_clicked();
    void newConnection();
    void removeConnection();
      void newMessage(QTcpSocket *sender, const QString &message);

signals:
    void disconnected();
private:
    Ui::MainWindow *ui;
    QTcpServer * m_server;
    QVector<QTcpSocket*> m_clients;
    QHash<QTcpSocket*, QByteArray> m_receivedData;
private:

void readyRead();
};

#endif // MAINWINDOW_H
