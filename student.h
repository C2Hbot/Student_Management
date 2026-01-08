#ifndef STUDENT_H
#define STUDENT_H
#include <QString> // Qt字符串类型，适配界面

class Student {
private:
    // 学生信息（私有成员，通过构造/接口访问）
    QString m_name;    // 姓名
    QString m_gender;  // 性别
    QString m_id;      // 学号
    double m_score;    // 总成绩
    QString m_grade;   // 等级（由成绩计算）

public:
    // 构造函数：创建学生时直接传入信息，自动算等级
    Student() : m_name(""), m_gender(""), m_id(""), m_score(0.0) {
        calculategrade(); // 初始成绩0.0→等级F
    }
    Student(const QString& name, const QString& gender, const QString& id, double score)
        : m_name(name), m_gender(gender), m_id(id), m_score(score) {
        calculategrade(); // 初始化时自动计算等级
    }

    //提供接口获取私有信息（用于统计）
    QString getgrade() const { return m_grade; } // 获取等级
    double getscore() const { return m_score; }  // 获取成绩
    QString getname() const { return m_name; }  //获取姓名
    QString getgender() const { return m_gender; }//获取性别
    QString getid() const { return m_id; }//获取学号

    void calculategrade();// 自动按成绩计算等级

    void setname(const QString&name);
    void setgender(const QString&gender);
    void setid(const QString&id);
    bool setscore(const double&score);
};

#endif // STUDENT_H
