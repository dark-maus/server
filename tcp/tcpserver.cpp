#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)//QObject(parent)
{
    qDebug() << this << "create on " << QThread::currentThread();
}

TcpServer::~TcpServer()
{
    qDebug() << this << "destroing";
}

bool TcpServer::listen(const QHostAddress &address, quint16 port)
{
        if(!QTcpServer::listen(address, port)) return false;
        m_thread = new QThread(this);
        m_connections = new TcpConnections();

        connect(m_thread, &QThread::started, m_connections, &TcpConnections::start, Qt::QueuedConnection);
        connect(this, &TcpServer::accepting, m_connections, &TcpConnections::accept, Qt::QueuedConnection);
        connect(this, &TcpServer::finished, m_connections, &TcpConnections::quit, Qt::QueuedConnection);
        connect(m_connections, &TcpConnections::finished, this, &TcpServer::complete, Qt::QueuedConnection);

        m_connections->moveToThread(m_thread);
        m_thread->start();

        return true;
}

void TcpServer::close()
{
    qDebug() << this << "closing server ";
    emit finished();
    QTcpServer::close();
}

qint64 TcpServer::port()
{
    if(isListening())
    {
        return this->serverPort();
    }
    else
    {
        return 1000;
    }
}

void TcpServer::incomingConnection(qintptr descriptor)
{
    qDebug() << this << "attempting to acept connection " << descriptor;
    TcpConnecton *connection = new TcpConnecton();
    accept(descriptor, connection);

}

void TcpServer::accept(qintptr descriptor, TcpConnecton *connection)
{
    qDebug() << this << "acepting te conection" << descriptor;

    connection->moveToThread(m_thread);
    emit accepting(descriptor, connection);
}

void TcpServer::complete()
{
    if(!m_thread)
    {
        qWarning() << this << "exitnig complete ther was no thred " ;
        return;
    }

    qDebug() << this << "complete called destronig threde";

    delete m_connections;

    qDebug() << this << "Quiting thread";

    m_thread->quit();
    m_thread ->wait();

    delete m_thread;

    qDebug()<< this << "complete";
}


