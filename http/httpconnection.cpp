#include "httpconnection.h"

HttpConnection::HttpConnection(QObject *parent) : TcpConnecton(parent)
{
    qDebug() <<this << "Creted httpconnection";

    m_mime.insert("txt","text/plain");
    m_mime.insert("htm","text/html");
    m_mime.insert("html","text/html");
    m_mime.insert("js","text/javascript");
    m_mime.insert("rtf","text/rtf");
    m_mime.insert("jpg","image/jpg");
    m_mime.insert("jpeg","image/jpeg");
    m_mime.insert("gif","image/gif");
    m_mime.insert("png","image/png");
    m_mime.insert("avi","video/avi");
    m_mime.insert("mpg","video/mpg");
    m_mime.insert("mpeg","video/mpeg");
    m_mime.insert("mp4","video/mp4");
    m_mime.insert("mkv","video/x-matroska");
    m_mime.insert("qt","video/quicktime");
    m_mime.insert("webm","video/webm");
    m_mime.insert("wmv","video/x-ms-wmv");
    m_mime.insert("flv","video/x-flv");
    m_mime.insert("mp3","audio/mp3");
    m_mime.insert("wav","audio/vnd.wav");
    m_mime.insert("tar","application/x-tar");
    m_mime.insert("zip","application/zip");
    m_mime.insert("pdf","application/pdf");
    m_mime.insert("xml","text/xml");
}

HttpConnection::~HttpConnection()
{
    qDebug() << this << "destroied ";
}

QString HttpConnection::root()
{
    return m_root;
}

void HttpConnection::setRoot(QString path)
{
    qDebug() <<this << "root set to: "<< path;
    m_root = path;
}

int HttpConnection::rate()
{
    return m_rate;
}

void HttpConnection::setRate(int value)
{
    qDebug() <<this << "Rate set to: "<< value;
    m_rate = value;
}

void HttpConnection::processGet(QByteArray data)
{
    qDebug() <<this << "Procesing GET ";
    QByteArray eoh;
    eoh.append("\r\n\r\n");

    QString header = m_request.value("svr_header", "");
    header.append(data);
    m_request.insert("svr_header", header);

    if(header.contains(eoh))
    {
        QStringList lines = header.split("\r\n");
        for(int i = 0; i < lines.count(); i++)
        {
             QString line= lines.at(i);

          if(i==0)
          {
            QStringList items = line.split(" ");
            if(items.count() >= 1) m_request.insert("http_method", items.at(0).trimmed());

            if(items.count() >= 2)
             {
                QString path = items.at(1).trimmed();
                path = QUrl::fromPercentEncoding(path.toLatin1());
                m_request.insert("http_path", path);
              }

              if(items.count() >= 3) m_request.insert("http_version", items.at(2).trimmed());
          }
          else
          {

              if(!line.contains(":") && !line.isEmpty())
              {
                  qWarning() << this << "Skiping line due to mising ':' " << line;
                  continue;
              }

              int pos  = line.indexOf(":");
              QString key = line.mid(0,pos);
              QString value = line.mid(pos+1);
              m_request.insert(key.trimmed(), value.trimmed());
          }
        }
        //attep to handle to the tequest
        handleRequest();
    }

}

void HttpConnection::handleRequest()
{
    qDebug() << this << "handle the request ";

    QString file = m_request.value("http_path","");

    //Skriptqut qny directory jumps
   file = file.replace("..","");

   QString actualFile = m_root + file;
   QFileInfo fi(actualFile);
   QByteArray response;

   //If int is a dirtctory, check for index.thml
   if(fi.isDir())
   {
       qDebug() << this << "client is requesteing a directory....";
       QString indexFile = actualFile + "index.html";
       QFileInfo fIndex(indexFile);

       if(fIndex.exists())
       {
           qDebug() << this << "setting / to /index.html";
           fi.setFile(indexFile);
       }
   }

   //send the file if it exists
   if(fi.exists() && fi.isFile())
   {
     //YES it existis
       QString mime = getMimeType(fi.fileName());
       qDebug() << this << "sending file: "<< fi.filePath();
       m_response.insert("code", "200");
       m_response.insert("path", fi.filePath());

       response.append("HTTP/1.1 200 OK\r\n");
       response.append("Content-Type:" + mime + "\r\n");
       response.append("Content-Length: " + QString::number(fi.size())+ "\r\n");
       response.append("Connection : close \r\n");
       response.append("\r\n");
   }
   else
   {
       //no it dont Exist
        m_response.insert("code", "404");

       response.append("HTTP/!1.1 404 NOT FOUND\r\n");
       response.append("Connection: close \r\n");
       response.append("\r\n");
   }

   m_socket->write(response);
}

void HttpConnection::sendFile(QString file)
{
    if(!m_socket) return;

    qDebug() << this << "sending file "<< file;

    m_file = new QFile(file,this);
    m_transfer = new RateTransfer(this);

    qDebug() <<this << "Creadet: " << m_transfer;

    connect(m_transfer, &RateTransfer::started, this, &HttpConnection::started);
    connect(m_transfer,&RateTransfer::finished,this, &HttpConnection::finished);
    connect(m_transfer,&RateTransfer::error,this, &HttpConnection::transferError);
    connect(m_transfer,&RateTransfer::transfered,this, &HttpConnection::transfered);

    if(!m_file->open(QFile::ReadOnly))
    {
            qWarning() << "colnd not open file: " << file;
            m_socket->close();
    }

    m_transfer->setSource(m_file);
    m_transfer->setDestination(m_socket);
    m_transfer->setRate(m_rate);
    m_transfer->setSize(1024);

    qDebug() << this << "starting file transfer ...";
    m_response.remove("code");

    m_transfer->start();

}

QString HttpConnection::getMimeType(QString path)
{
    int pos = path.indexOf(".");
    if(pos <= 0) return "application/octet-stream";

    QString ext =path.mid(pos +1).toLower();
    if(m_mime.contains(ext))
    {
        return m_mime.value(ext);
    }

    return "application/octet-stream";
}

void HttpConnection::connected()
{
    qDebug() <<this << "connected" << getSocket();
}

void HttpConnection::disconnected()
{
     qDebug() <<this << "disconnected" << getSocket();
}

void HttpConnection::readyRead()
{
    if(!m_socket) return;
     qDebug() <<this << "readyRead" << m_socket;
    QByteArray data = m_socket->readAll();
    processGet(data);
}

void HttpConnection::bytesWritten(qint64 bytes)
{
    if(!m_socket) return;

    QString code = m_response.value("code", "");

    if(!code.isEmpty())
    {
    qDebug() << this << "code = "<< code;

    }

    if(code == "200")
    {
        QString file = m_response.value("path","");
        qDebug() << this << "Apttenting to send file  = "<< file;
        sendFile(file);
        //close socket when done
    }

    if(code =="404")
    {
        m_socket->close();
        return;
    }
}

void HttpConnection::stateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << this << "stateChanged " << m_socket <<socketState;
}

void HttpConnection::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << this << "ERROR   " << m_socket <<socketError;
}

void HttpConnection::started()
{
    qDebug()<< this << "file transfer strted ";
}

void HttpConnection::transfered(qint64 bytes)
{
    qDebug() <<this << "file transfered " << bytes;

}

void HttpConnection::finished()
{
    qDebug() <<this << "file transfer fnished ";
    m_file->close();
    m_socket->close();
}

void HttpConnection::transferError()
{
    qDebug() << this << "file transfer error : "<< m_transfer ->errorString();
    m_file->close();
    m_socket->close();
}
