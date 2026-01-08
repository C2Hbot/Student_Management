#ifndef STATISTIC_PAGE_H
#define STATISTIC_PAGE_H

#include <QWidget>

namespace Ui {
class statistic_page;
}

class statistic_page : public QWidget
{
    Q_OBJECT

public:
    explicit statistic_page(QWidget *parent = nullptr);
    ~statistic_page();

private:
    Ui::statistic_page *ui;
public:
    void setTotalCount(int count);                  // 总人数
    void setPassRate(double rate);                  // 及格率（百分比）
    void setExcellentRate(double rate);             // 优良率（百分比）
    void setGradeCount(const QString& grade, int count, double rate); // 各等级人数及比例（按成绩设置）
};

#endif // STATISTIC_PAGE_H
