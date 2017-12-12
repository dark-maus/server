#include "ratetransfer.h"

RateTransfer::RateTransfer(QObject *parent) : QObject(parent)
{
    qDebug() << this << "Creatd";
    setDefaults();
}

RateTransfer::~RateTransfer()
{
    qDebug() << this << "Destroyed";
}

int RateTransfer::rate()
{
    return m_rate;
}

void RateTransfer::setRate(int value)
{
    m_rate =value;
    qDebug() << this << "Rate set to "<< value;
}

int RateTransfer::size()
{
    return m_size;
}

void RateTransfer::setSize(int value)
{
    m_size = value;
    qDebug() << this << "Size set to "<< value;
}

qint64 RateTransfer::maximum()
{
    return m_maximum;
}

void RateTransfer::setMaximum(qint64 value)
{
    m_maximum = value;
    qDebug() << this << "maximum set to "<< value;
}

QIODevice *RateTransfer::source()
{
    return m_source;
}

void RateTransfer::setSource(QIODevice *device)
{
    m_source = device;
    qDebug() << this << "Source set to " << device;
    if(m_source->isSequential()) connect(m_source, &QIODevice::readyRead, this, &RateTransfer::readyRead);

}

QIODevice *RateTransfer::destination()
{
    return m_destination;
}

void RateTransfer::setDestination(QIODevice *device)
{
    m_destination = device;
    qDebug() << this << "Destination set to " << device;
      if(m_destination->isSequential()) connect(m_source, &QIODevice::bytesWritten, this, &RateTransfer::bytesWritten);
}

bool RateTransfer::isTransfering()
{
    return m_transfering;
}

QString RateTransfer::errorString()
{
    return m_error;
}

void RateTransfer::setDefaults()
{
    qDebug() << this << "Setting the deflaut";

    m_rate = 0;
    m_size = 1024;
    m_maximum = 0;
    m_transfering = false;
   m_transfered =0;
    m_source = 0;
    m_destination = 0;
    m_error = "";
    m_scheduled = false;
    m_timer.setInterval(5);

}

bool RateTransfer::checkDevices()
{
    if(!m_source)
    {
        m_transfering = false;
        m_error = "No destination device ";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    if(!m_destination)
    {
        m_transfering = false;
        m_error = "No source device ";
        qDebug() << this << m_error;
        emit error();
        return false;

    }

    if(!m_source->isOpen() || !m_source->isReadable())
    {
        m_transfering = false;
        m_error = "Source device is not open or redible !!";
        qDebug() << this << m_error;
        emit error();
        return false;

    }

    if(!m_destination->isOpen() || !m_destination->isWritable())
    {
        m_transfering = false;
        m_error = "Source device is not open or Wirable !!";
        qDebug() << this << m_error;
        emit error();
        return false;

    }

    return true;
}

bool RateTransfer::checkTransfer()
{
    if(!m_transfering)
    {
        m_error = "Not transfering aborting  !!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    if(m_transfered >= m_rate) //if(m_transfering >= m_rate)
    {
        m_error = "Rate exeeded, not allowed to transfer !!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }
    return true;
}

void RateTransfer::scheduleTransfer()
{
    qDebug() << this << "shceduleTranfer";

    if(m_scheduled)
    {
        qWarning() << this << "Exiting scheduleTransfer due to : waiting on timer ";
        return;
    }

    if(!m_transfering)
    {
        qWarning() << this << "Exiting scheduleTransfer due to : not transfering ";
        return;
    }

    if(m_source->bytesAvailable() <= 0)
    {
        qWarning() << this << "Exiting scheduleTransfer due to : not bytes available to read";
        return;
    }

    int prediction = m_transfered + m_size;
    if(prediction <= m_rate)
    {
        qDebug() << this << "calling transfer from schedule Transfer ";
        transfer();
    }
    else
    {
        int current = QTime::currentTime().msec();
        int delay = 1000 - current;
        qDebug() <<this<< "Rate limits (" << m_rate << ") exeeded in prediction (" <<m_transfered << " to "<< prediction << "). delaying transfer for " << delay << " ms ";
       m_transfered =0;
        m_scheduled = true;
        m_timer.singleShot(delay, this, &RateTransfer::transfer);
        //m_timer.singleShot(delay,this,&RateTransfer::transfer);

    }
}

void RateTransfer::start()
{
     qDebug() << this << "Start called to sie wyswietla";
     if(m_transfering)
     {
         qDebug() << "alredy transfering";
         return;
     }

     m_error = "";
     if(!checkDevices()) return;

     m_transfering = true;
    m_transfered = 0;
     emit started();


     if(!m_source->isSequential() && m_source->bytesAvailable() > 0)
     {
         qDebug()<< this << "trting transfer by caling schuleTransfer ";
         scheduleTransfer();
     }

}

void RateTransfer::stop()
{
    qDebug()<< this << "stoping transfer ";
    m_timer.stop();
    m_transfering = false;

}

void RateTransfer::transfer()
{
    m_scheduled = false;
    qDebug()<< this << "transfering at maximum of " << m_rate << " bytes per secund ";
    m_error ="";

    if(!checkDevices()) return;
    if(!checkTransfer()) return;

    qDebug()<< this << "reading from source";
    QByteArray buffer;
    buffer = m_source->read(m_size);

    qDebug() << this << " writting to destination " << buffer.length();
    m_destination -> write(buffer);
   m_transfered += buffer.length();
    emit transfered(m_transfered);


    if(m_maximum > 0 &&m_transfered >= m_maximum)
    {
        qDebug() << this << " Stoping due to maximum imits reached";
        emit finished();
        stop();
    }

    if(!m_source->isSequential() && m_source->bytesAvailable() == 0)
    {
        qDebug() << this << "Stoping due to end of file ";
        emit finished();
        stop();
    }

    if(m_transfering == false) return;
    if(!m_source->isSequential() && m_source->bytesAvailable() > 0)
    {
        qDebug() << this << "Source stil has bytes  schulding a transfered ";
        scheduleTransfer();
    }

}

void RateTransfer::readyRead()
{
    qDebug() << this << "redyRead() signaled ";
    scheduleTransfer();
}

void RateTransfer::bytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    qDebug() << this << "bytesWritten signaled ";
    scheduleTransfer();
}
