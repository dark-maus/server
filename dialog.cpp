/*
funkcje gui
*/

#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->btnStop->setEnabled(false); //na poczatku zablokowanie przycisku stop
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_btnStart_clicked()
{
    if(m_server.listen(QHostAddress::Any, ui->spinBox->value()))
    {
        qDebug() <<this <<" Server Started !!!";
        m_server.setRate(1536000);
        m_server.setRoot("C:/Code/MyServer/website");
        setStarted(true);

    }
    else
    {
        qCritical() << m_server.errorString();
        setStarted(false);
    }
    setStarted(true); //klikniecie przycisku -> true
}

void Dialog::on_btnStop_clicked()
{
    m_server.close();
     setStarted(false);
}
/* aktywacja i deaktywacja przycisu
 * jak kilkniety jest "otzycis stat"
 *  to aktywny jest tylko przycis "stop" */
void Dialog::setStarted(bool started)
{
    if(started)
    {
        ui->btnStart->setEnabled(false);
        ui->btnStop->setEnabled(true);
    }
    else
    {
        ui->btnStart->setEnabled(true);
        ui->btnStop->setEnabled(false);
    }
}
