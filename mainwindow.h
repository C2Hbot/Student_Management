#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>   // 弹窗头文件
#include <QFile>         // 读取txt需用
#include <QFileDialog>   //选择文件
#include <QTextStream>  //数据流处理
#include <QSqlDatabase>  // 数据库需用
#include <QSqlQuery>     // 数据库查询需用
#include <QSqlError>     //数据库读取报错
#include "statistic_page.h"//统计页面
#include <QDialog>       // 必须加：QDialog的头文件
#include <QHBoxLayout>   // 布局用
#include <QLabel>        // 显示提示文本用
#include <QPushButton>   // 按钮用
#include <vector>        //学生容器类
#include "student.h"     //学生类
#include "QTableWidget"  //展示学生信息用
#include <QDataStream>  // 二进制序列化用
#include <QCoreApplication>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private://（数据元素）

    std::vector<Student*> m_studentList;// 动态管理学生对象：vector存Student指针（统计用）

    std::vector<int> m_displayIndices;// 主页面显示数组（索引存学生序号，显示用）

    statistic_page* m_statPage = nullptr;// 统计页面指针（创建行，用于更新统计信息）

    bool m_isEditMode = true;// 模式标记：true=修改模式，false=只读模式

private slots://信号函数
    //按钮函数
    void on_read_button_clicked();//读取数据的按钮函数
    void readDataFromTxt();   // 从txt读取
    void readDataFromDB();    // 从数据库读取
    void on_statistic_button_clicked();//打开统计页面按钮
    void on_insert_button_clicked();//插入学生按钮
    void on_delete_button_clicked();//删除学生按钮
    void on_change_button_clicked();//修改权限按钮
    void on_search_button_clicked();//查询按钮
    void on_write_button_clicked();//写入数据按钮
    void WriteDataToTxt();   //写入TXT文件
    void WriteDataToDB();    //写入SQLite数据库
    //其他信号函数
    void on_stu_info_itemChanged(QTableWidgetItem* item);//添加学生的辅助函数（创建一个新行并初始化）

private://辅助函数
    void calculate_statistic();  //新增统计计算函数
    void saveToBinaryFile();     //保存学生数据到二进制文件
    bool loadFromBinaryFile();   //从二进制文件加载学生数据
    void refreshTable();         //刷新表格显示
    void showAllStudents();      //显示所有学生
    bool isStudentInfoComplete(Student* stu);//校验学生信息是否完整
    bool isIdExists(const QString& id, int currentRow = -1);//查找学号是否重复

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
