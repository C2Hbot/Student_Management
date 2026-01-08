#include "student.h"
#include <QMessageBox>
// 内部函数：根据成绩计算等级
void Student::calculategrade() {
    if (m_score >= 90) m_grade = "A";
    else if (m_score >= 80) m_grade = "B";
    else if (m_score >= 70) m_grade = "C";
    else if (m_score >= 60) m_grade = "D";
    else m_grade = "F";
}

void Student::setname(const QString&name)//设置名字
{
    m_name=name;
}

void Student::setgender(const QString&gender)//设置性别
{
    m_gender=gender;
}

void Student::setid(const QString&id)//设置学号
{
    m_id=id;
}

bool Student::setscore(const double&score)//设置成绩
{
    if(score>100||score<0)
    {
        QMessageBox::warning(nullptr,"成绩错误", "成绩必须在0-100之间！");
            return false;
    }
    else
    {
        m_score=score;
        calculategrade();
        return true;
    }
}
