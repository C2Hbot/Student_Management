#include "statistic_page.h"
#include "ui_statistic_page.h"

statistic_page::statistic_page(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::statistic_page)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);//设置为独立窗口
}

statistic_page::~statistic_page()
{
    delete ui;
}

void statistic_page::setTotalCount(int count)
{
    ui->sum->setText(QString("总人数: %1人").arg(count));
}

void statistic_page::setPassRate(double rate)
{
    ui->pass->setText(QString("及格率: %1%").arg(rate, 0, 'f', 1));
}

void statistic_page::setExcellentRate(double rate)
{
    ui->EX->setText(QString("优良率: %1%").arg(rate, 0, 'f', 1));
}

void statistic_page::setGradeCount(const QString& grade, int count, double rate)
{
    if (grade == "A") {
        ui->A->setText(QString("等级A(90-100): %1人 (%2%)").arg(count).arg(rate, 0, 'f', 1));
    } else if (grade == "B") {
        ui->B->setText(QString("等级B(80-89): %1人 (%2%)").arg(count).arg(rate, 0, 'f', 1));
    } else if (grade == "C") {
        ui->C->setText(QString("等级C(70-79): %1人 (%2%)").arg(count).arg(rate, 0, 'f', 1));
    } else if (grade == "D") {
        ui->D->setText(QString("等级D(60-69): %1人 (%2%)").arg(count).arg(rate, 0, 'f', 1));
    } else if (grade == "F") {
        ui->F->setText(QString("等级F(0-59): %1人 (%2%)").arg(count).arg(rate, 0, 'f', 1));
    }
}
