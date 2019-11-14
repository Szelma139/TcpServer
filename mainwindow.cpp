#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>
#include <QChar>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_server(new QTcpServer(this))
{
    ui->setupUi(this);

    if(!m_server->listen(QHostAddress::LocalHost, 52693))
    {
        ui->log->appendPlainText(("Failure while starting server: "  + m_server->errorString()));
        return;
    }
    connect(m_server, &QTcpServer::newConnection,this, &MainWindow::newConnection);
    ui->address->setText("Address: " + m_server->serverAddress().toString());
    ui->port->setText("Port: " + QString::number(m_server->serverPort()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_disconnectClients_clicked()
{

}

void MainWindow::newConnection()
{
    while (m_server->hasPendingConnections()) //jesli jest wiecej pending connectionow
    {
        QTcpSocket * con = m_server->nextPendingConnection(); //serwer obsluz kolejna
        m_clients << con; //wrzuc socket do vectora socketow
        ui->disconnectClients->setEnabled(true);
        connect(con, SIGNAL(disconnected()),this, SLOT(removeConnection()));
        connect(con, &QTcpSocket::readyRead, this, &MainWindow::readyRead);
        ui->log->appendPlainText(tr("* New Connection: %1, port %2\n")
                                 .arg(con->peerAddress().toString())
                                 .arg(QString::number(con->peerPort())));
    }
    qDebug() << "Mamy nowe polaczenie";



}


void MainWindow::removeConnection()
{
    if (QTcpSocket * con = qobject_cast<QTcpSocket*>(sender()))
    { //wskaznik na obiekt ktory wywolal sygnal
        ui->log->appendPlainText(
                    tr("* Connection removed: %1, port %2 \n")
        .arg(con->peerAddress().toString()) //na andresie
                .arg(QString::number(con->peerPort()))); //na porcie
        m_clients.removeOne(con);
        con->deleteLater(); //wywal konkretne polaczenie socketa
        ui->disconnectClients->setEnabled(!m_clients.isEmpty());
    }
}

void MainWindow::newMessage(QTcpSocket *sender, const QString &message)
{
    ui->log->appendPlainText(tr("Sending message: %1").arg(message));
    QByteArray messageArray = message.toUtf8(); //wrzuc stringa do messagearray --> konwersja
    messageArray.append(23); //dopsiz 23 zeby bylo wiadomo gdzie sie konczy pakiet
    for(QTcpSocket *socket: m_clients) {
        if (socket->state() == QAbstractSocket::ConnectedState) {
          //  if (socket->isOpen())
            qDebug() << "Socket jest otwarty";
           socket->write(messageArray);

           // qDebug() << "Data written " << socket->write(messageArray.data());
          //  qDebug() << "Bytes available " << socket->bytesAvailable();
            //emit readyRead();
        }
    }
    Q_UNUSED(sender) //sender to normalnie sender() metoda wywolujaca obiekt ktory dal sygnal,
    //UQUNUSED pozwala uzywac nazw zastrzezonych dla np. bibliotek
}

void MainWindow::readyRead()
{
    QTcpSocket * socket = qobject_cast<QTcpSocket*>(sender()); //dla sygnalu z socketa (ktoregokolwiek_
   if(!socket)
   {
       return;
   }

    QByteArray &buffer  = m_receivedData[socket]; //wpisz do buffera okreslony socket
    buffer.append(socket->readAll()); //dopisz cala wartosc zczytana z socketu
    while(true)
    {
        int endIndex = buffer.indexOf(23); //end index to taki na ktorym jest 23 - bo to koniec wiadomosc a wiec koniec pakietu

      if(endIndex < 0){ //w drugim przypadku endIndex = -1 czyli false bo go wcale niema a zmienna buffer jest lokalna
          //i dlatego tracona jest jej wartosc
      //jesli mniejszy niz 0 to koniec
          break;
    }
qDebug() << "Ready read";
        QString message = QString::fromUtf8(buffer.left(endIndex)); //message - wsztstko na lewo od
        buffer.remove(0,endIndex+1);
        newMessage(socket, message); //wyslij message socketem //endindex bierze wszystko od lewej do end index

    }
    //w tym miejscu wartosc buffera zostaje zgubiona
    //bo wychodzimy poza zakres funkcji i przy kolejnym odpaleniu buffer bedzie = " "
}
