#include "mainwindow.h"
#include "ui_mainwindow.h"
#define BINARY_FILE_PATH QCoreApplication::applicationDirPath() + "/students.dat"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stu_info->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//让表格所有列等比例拉伸
    // 初始化模式为修改模式
    m_isEditMode = true;
    // 初始化表格编辑权限（修改模式下可单击编辑）
    ui->stu_info->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::CurrentChanged);
    if (loadFromBinaryFile()) {
        QMessageBox::information(this, "加载成功",
                                 QString("成功加载%1名学生的历史数据").arg(m_studentList.size()));
    } else {
        QMessageBox::information(this, "提示", "未找到历史数据文件，将创建新数据");
    }

    // 初始化显示所有学生
    showAllStudents();
}

MainWindow::~MainWindow()
{
    // 退出时保存二进制文件
    saveToBinaryFile();

    // 释放学生对象内存
    for (Student* stu : m_studentList) {
        delete stu;
    }
    m_studentList.clear();
    m_displayIndices.clear();

    // 清理表格
    int rowCount = ui->stu_info->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < 5; ++j) {
            QTableWidgetItem* item = ui->stu_info->takeItem(i, j);
            delete item;
            item = nullptr;
        }
        ui->stu_info->removeRow(i);
    }

    // 清理统计页面
    if (m_statPage) {
        delete m_statPage;
        m_statPage = nullptr;
    }
    delete ui;
}

void MainWindow::saveToBinaryFile()//向二进制文件写入数据（退出系统自动执行）
{
    QFile file(BINARY_FILE_PATH);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "保存失败",
                             "二进制文件写入失败！\n原因：" + file.errorString());
        return;
    }

    QDataStream out(&file);
    //写入学生总数
    out << static_cast<qint32>(m_studentList.size());

    //逐个序列化学生对象
    for (Student* stu : m_studentList) {
        if (!stu) continue;
        // 序列化顺序：姓名、性别、学号、成绩（等级由成绩计算，无需保存）
        out << stu->getname() << stu->getgender() << stu->getid() << stu->getscore();
    }

    file.close();
    QMessageBox::information(this, "保存成功",
                             QString("已将%1名学生数据保存至：\n%2").arg(m_studentList.size()).arg(BINARY_FILE_PATH));
}

bool MainWindow::loadFromBinaryFile()//从二进制文件读数据（初始化自动执行）
{
    QFile file(BINARY_FILE_PATH);
    if (!file.exists()) {
        return false; // 文件不存在，返回false
    }

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "加载失败",
                             "二进制文件读取失败！\n原因：" + file.errorString());
        return false;
    }

    QDataStream in(&file);
    //读取学生总数
    qint32 totalCount;
    in >> totalCount;
    if (totalCount < 0) { // 数据格式错误
        file.close();
        QMessageBox::warning(this, "加载失败", "二进制文件格式错误（总数非法）");
        return false;
    }
    //逐个反序列化学生对象
    for (qint32 i = 0; i < totalCount; ++i) {
        QString name, gender, id;
        double score;
        // 按保存顺序读取
        in >> name >> gender >> id >> score;

        // 数据有效性校验
        if (name.isEmpty() || id.isEmpty() || score < 0 || score > 100) {
            continue; // 无效数据整行跳过
        }

        // 创建学生对象并加入列表
        Student* newStu = new Student(name, gender, id, score);
        m_studentList.push_back(newStu);
    }

    file.close();
    return m_studentList.size() > 0;
}

void MainWindow::showAllStudents()//显示所有学生
{
    //先清空索引数组
    m_displayIndices.clear();

    //遍历所有学生，只存入序号
    for (size_t i = 0; i < m_studentList.size(); ++i) {
        m_displayIndices.push_back(static_cast<int>(i));
    }

    //刷新表格（只展示，不修改）
    refreshTable();

    //显示全部时，若处于修改模式，恢复表格编辑权限
    if (m_isEditMode) {
        ui->stu_info->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::CurrentChanged);
    }
}

void MainWindow::refreshTable()//更新显示表格函数
{
    //清空表格（只清空展示，不删除m_studentList的数据）

    while (ui->stu_info->rowCount() > 0) {// 倒序删除行，避免索引错乱
        ui->stu_info->removeRow(0);
    }

    //只根据m_displayIndices的序号，从m_studentList读取数据展示
    //遍历索引数组，不修改m_studentList，不修改学生序号/信息
    for (size_t i = 0; i < m_displayIndices.size(); ++i) {
        // 取出学生在m_studentList中的序号
        int studentIndex = m_displayIndices[i];

        //序号有效，防止数组越界
        if (studentIndex < 0 || static_cast<size_t>(studentIndex) >= m_studentList.size()) {
            continue;
        }

        //从m_studentList中读取学生
        Student* stu = m_studentList[studentIndex];
        if (!stu) continue;

        //插入新行，填充学生信息
        int tableRow = static_cast<int>(i); // 表格的行号（和索引数组对应）
        ui->stu_info->insertRow(tableRow);

        //按列填充：姓名、性别、学号、成绩、等级
        ui->stu_info->setItem(tableRow, 0, new QTableWidgetItem(stu->getname()));
        ui->stu_info->setItem(tableRow, 1, new QTableWidgetItem(stu->getgender()));
        ui->stu_info->setItem(tableRow, 2, new QTableWidgetItem(stu->getid()));
        ui->stu_info->setItem(tableRow, 3, new QTableWidgetItem(QString::number(stu->getscore(), 'f', 1)));

        //等级列：不可编辑（始终只读，只能由成绩计算得到）
        QTableWidgetItem* gradeItem = new QTableWidgetItem(stu->getgrade());
        gradeItem->setFlags(gradeItem->flags() & ~Qt::ItemIsEditable);
        ui->stu_info->setItem(tableRow, 4, gradeItem);
    }
}

void MainWindow::on_read_button_clicked()//读取数据按钮
{
    //创建QDialog窗口作为选项框
    QDialog chooseDialog(this);
    chooseDialog.setWindowTitle("选择读取方式"); // 窗口标题
    QLabel *tipLabel = new QLabel("请选择数据读取方式：");//加提示文本
    QPushButton *txtBtn = new QPushButton("读取TXT文件");
    QPushButton *dbBtn = new QPushButton("读取数据库");//加两个按钮
    QHBoxLayout *layout = new QHBoxLayout(&chooseDialog); // 把布局直接绑定到窗口
    layout->addWidget(tipLabel);
    layout->addWidget(txtBtn);
    layout->addWidget(dbBtn);//用布局把文本和按钮排成竖列

    //定义变量记录“被点击的按钮”，初始为空
    QPushButton *clickedBtn = nullptr;
    connect(txtBtn, &QPushButton::clicked, &chooseDialog, [&]() {
        clickedBtn = txtBtn; // 点了TXT按钮，记录下来
        chooseDialog.close(); // 关闭窗口
    });
    connect(dbBtn, &QPushButton::clicked, &chooseDialog, [&]() {
        clickedBtn = dbBtn; // 点了数据库按钮，记录下来
        chooseDialog.close(); // 关闭窗口
    });

    chooseDialog.exec();//显示窗口（阻塞式）

    //判断被点击的按钮，执行对应逻辑
    if (clickedBtn == txtBtn) {
        readDataFromTxt(); // 读TXT
    } else if (clickedBtn == dbBtn) {
        readDataFromDB(); // 读数据库
    }
    //点叉号的话，clickedBtn还是空，不会执行任何逻辑
}

void MainWindow::readDataFromTxt()//从txt文件读数据
{
    //打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择TXT文件",
        QCoreApplication::applicationDirPath(), //初始路径为.exe运行目录
        "TXT文件 (*.txt)" //过滤文件类型
        );

    if (filePath.isEmpty()) {//用户取消选择
        return;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误",
                             "TXT文件打开失败！\n原因：" + file.errorString());
        return;
    }

    QTextStream in(&file);
    int successCount = 0; //记录成功导入的学生数

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); //读取一行并去除首尾空格
        if (line.isEmpty()) continue; //跳过空行

        //按英文逗号分割字段
        QStringList fields = line.split(",");
        // 校验字段数量（必须为4个）
        if (fields.size() != 4) {
            continue;
        }

        //提取字段并校验
        QString name = fields[0].trimmed();
        QString gender = fields[1].trimmed();
        QString id = fields[2].trimmed();
        bool isScoreValid;
        double score = fields[3].trimmed().toDouble(&isScoreValid);

        //校验成绩是否有效（0-100）
        if (!isScoreValid || score < 0 || score > 100) {
            continue;
        }
        //验证学号不重复
        if(isIdExists(id)){
            continue;
        }
        //校验姓名、学号不为空
        if (name.isEmpty() || id.isEmpty()) {
            continue;
        }

        //添加到m_studentList（追加，不覆盖）
        Student* newStu = new Student(name, gender, id, score);
        m_studentList.push_back(newStu);

        // //添加到表格（在末尾新建行）
        // int newRow = ui->stu_info->rowCount(); // 获取当前表格最后一行索引
        // ui->stu_info->insertRow(newRow);

        // //设置表格单元格内容
        // ui->stu_info->setItem(newRow, 0, new QTableWidgetItem(name));
        // ui->stu_info->setItem(newRow, 1, new QTableWidgetItem(gender));
        // ui->stu_info->setItem(newRow, 2, new QTableWidgetItem(id));
        // ui->stu_info->setItem(newRow, 3, new QTableWidgetItem(QString::number(score, 'f', 1)));

        // //等级列（自动计算）
        // QTableWidgetItem* gradeItem = new QTableWidgetItem(newStu->getgrade());
        // gradeItem->setFlags(gradeItem->flags() & ~Qt::ItemIsEditable);
        // ui->stu_info->setItem(newRow, 4, gradeItem);
        successCount++;
    }

    file.close();
    //创建提示框提示导入结果
    if (successCount > 0) {
        QMessageBox::information(this, "成功",
                                 QString("TXT数据导入完成！\n共导入%1名学生").arg(successCount));
        calculate_statistic(); // 更新统计信息
    } else {
        QMessageBox::information(this, "提示", "未导入任何学生数据");
    }
    showAllStudents();
}

void MainWindow::readDataFromDB()//从数据库读数据
{
    //弹出文件选择对话框，仅允许选择 .db 文件
    QString dbPath = QFileDialog::getOpenFileName(
        this,
        "选择要读取的SQLite数据库",//对话框标题
        QCoreApplication::applicationDirPath(), //初始路径：应用运行目录
        "SQLite数据库文件 (*.db)"       //仅显示.db 格式文件，用户无法选择其他类型
        );

    if (dbPath.isEmpty()) { //用户取消选择，直接返回
        return;
    }
    // 连接SQLite数据库
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        //替换驱动为SQLite
        db = QSqlDatabase::addDatabase("QSQLITE");
        //数据库文件路径：应用程序同级目录下的student_manage.db（不存在则自动创建）
        db.setDatabaseName(dbPath);
    }

    //打开数据库
    if (!db.open()) {
        QMessageBox::warning(this, "数据库连接失败",
                             "无法连接到SQLite数据库！\n错误信息：" + db.lastError().text());
        return;
    }

    //自动创建学生表，模板如下
    QSqlQuery createQuery;
    QString createSql = R"(
        CREATE TABLE IF NOT EXISTS student_info (
            name VARCHAR(50) NOT NULL,
            gender VARCHAR(10) NOT NULL,
            id VARCHAR(20) NOT NULL UNIQUE,
            score DOUBLE NOT NULL,
            PRIMARY KEY (id)
        );
    )";
    if (!createQuery.exec(createSql)) {
        QMessageBox::warning(this, "建表失败",
                             "自动创建学生表出错！\n错误信息：" + createQuery.lastError().text());
        db.close();
        return;
    }

    //数据查询
    QSqlQuery query;
    QString sql = "SELECT name, gender, id, score FROM student_info;";
    if (!query.exec(sql)) {
        QMessageBox::warning(this, "数据查询失败",
                             "查询学生数据出错！\n错误信息：" + query.lastError().text());
        db.close();
        return;
    }

    //数据合法性判断
    int successCount = 0;
    QString errorMsg;
    while (query.next()) {
        QString name = query.value(0).toString().trimmed();
        QString gender = query.value(1).toString().trimmed();
        QString id = query.value(2).toString().trimmed();
        double score = query.value(3).toDouble();

        if (name.isEmpty() || id.isEmpty()) {
            errorMsg += QString("学号「%1」：姓名或学号不能为空，跳过导入\n").arg(id);
            continue;
        }
        if (score < 0 || score > 100) {
            errorMsg += QString("学号「%1」：成绩%2无效，跳过导入\n").arg(id).arg(score);
            continue;
        }
        if (isIdExists(id)) {
            errorMsg += QString("学号「%1」：已存在，跳过导入\n").arg(id);
            continue;
        }

        // 添加到本地列表
        Student* newStu = new Student(name, gender, id, score);
        m_studentList.push_back(newStu);
        successCount++;
    }

    db.close();
    QString msg = QString("SQLite数据导入完成！\n成功导入%1名学生\n").arg(successCount);
    if (!errorMsg.isEmpty()) {
        msg += "失败信息：\n" + errorMsg;
    }
    QMessageBox::information(this, "导入结果", msg);
    showAllStudents();
    calculate_statistic();//导入数据后刷新表格
}

void MainWindow::on_change_button_clicked()//权限更改按钮
{
    //切换模式
    m_isEditMode = !m_isEditMode;

    //根据新模式，设置widget（stu_info）的编辑权限
    if (m_isEditMode)
    {
        //修改模式下，单击单元格即可进入编辑状态
        ui->stu_info->setEditTriggers(QAbstractItemView::CurrentChanged);
        //确保单击选中后可编辑
        ui->stu_info->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::CurrentChanged);

        //修改模式下，启用几个被禁用的按钮
        ui->read_button->setEnabled(true);        //启用on_read_button_clicked()
        ui->insert_button->setEnabled(true);      //启用on_insert_buttom_clicked()
        ui->delete_button->setEnabled(true);      //启用on_delete_buttom_clicked()
        ui->write_button->setEnabled(true);       //启用on_delete_buttom_clicked()

        //更新界面模式提示
        ui->mode_text->setText("当前模式：修改模式");

        //弹出模式切换提示
        QMessageBox::information(this, "模式切换", "已切换为【修改模式】，单击表格单元格可编辑，所有功能按钮已启用！");
        showAllStudents();//修改模式显示全部学生
    }
    else
    {
        //只读模式下，禁止所有编辑操作（无法修改任何单元格）
        ui->stu_info->setEditTriggers(QAbstractItemView::NoEditTriggers);

        // 步骤3：只读模式下，禁用几个按钮（按钮禁用后，对应点击函数无法触发）
        ui->read_button->setEnabled(false);       //禁用on_read_button_clicked()
        ui->insert_button->setEnabled(false);     //禁用on_insert_buttom_clicked()
        ui->delete_button->setEnabled(false);     //禁用on_delete_buttom_clicked()
        ui->write_button->setEnabled(false);      //禁用on_write_buttom_clicked()
        //更新界面模式提示
        ui->mode_text->setText("当前模式：只读模式");
        //弹出模式切换提示
        QMessageBox::information(this, "模式切换", "已切换为【只读模式】，表格不可编辑，读取/添加/删除/写入功能已禁用！");
    }
}

void MainWindow::on_statistic_button_clicked()//打开统计页按钮
{
    if (!m_statPage) {//仅在首次使用时创建
        m_statPage = new statistic_page(this);
    }
    calculate_statistic(); //计算结果更新到m_statPage
    m_statPage->show(); //显示复用的对象
}

void MainWindow::on_insert_button_clicked()//添加学生按钮
{
    //创建学生对象（动态内存分配）
    Student* newStu = new Student("","","",0.0);
    if (newStu == nullptr) {
        QMessageBox::critical(this, "错误", "创建学生对象失败！");
        return;
    }
    m_studentList.push_back(newStu); //把对象加入数组
    showAllStudents();
    calculate_statistic();
}

void MainWindow::on_delete_button_clicked()//删除学生按钮
{
    //获取当前选中的行索引
    int selectedRow = ui->stu_info->currentRow();

    //边界判断：没有选中行或索引超出范围
    if (selectedRow < 0 || static_cast<size_t>(selectedRow) >= m_studentList.size()) {
        QMessageBox::warning(this, "操作提示", "请先选中要删除的学生行！");
        return;
    }

    //弹窗二次确认
    QMessageBox::StandardButton result = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除第%1行的学生信息吗？").arg(selectedRow + 1),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (result != QMessageBox::Yes) {
        return; // 用户取消删除
    }

    //释放动态分配的Student对象内存
    Student* stuToDelete = m_studentList[selectedRow];
    delete stuToDelete;
    stuToDelete = nullptr; //避免野指针

    //从学生数组m_studentList中移除该元素
    m_studentList.erase(m_studentList.begin() + selectedRow);

    //更新统计信息
    showAllStudents();
    calculate_statistic();
}

void MainWindow::on_write_button_clicked()//写入数据按钮
{
    QDialog chooseDialog(this);//创建QDialog窗口
    chooseDialog.setWindowTitle("选择写入方式"); // 窗口标题
    QLabel *tipLabel = new QLabel("请选择数据写入方式：");//加提示文本
    QPushButton *txtBtn = new QPushButton("写入TXT文件");
    QPushButton *dbBtn = new QPushButton("写入数据库");//加两个按钮

    QHBoxLayout *layout = new QHBoxLayout(&chooseDialog); // 把布局直接绑定到窗口
    layout->addWidget(tipLabel);
    layout->addWidget(txtBtn);
    layout->addWidget(dbBtn);//用布局把文本和按钮排成竖列

    QPushButton *clickedBtn = nullptr;//定义变量记录“被点击的按钮”，初始为空

    connect(txtBtn, &QPushButton::clicked, &chooseDialog, [&]() {
        clickedBtn = txtBtn; //点了TXT按钮，记录下来
        chooseDialog.close(); //关闭窗口
    });
    connect(dbBtn, &QPushButton::clicked, &chooseDialog, [&]() {
        clickedBtn = dbBtn; //点了数据库按钮，记录下来
        chooseDialog.close(); //关闭窗口
    });

    //显示窗口（阻塞式）
    chooseDialog.exec();

    if (clickedBtn == txtBtn) {
        WriteDataToTxt(); //读TXT
    } else if (clickedBtn == dbBtn) {
        WriteDataToDB(); //读数据库
    }
    //判断被点击的按钮，执行对应逻辑,点叉号的话，clickedBtn还是空，不会执行任何逻辑
}

void MainWindow::WriteDataToTxt()//向选定txt文件写入数据
{
    //打开文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "选择TXT文件保存位置",
        QCoreApplication::applicationDirPath(), //初始路径为运行目录
        "TXT文件 (*.txt)" //过滤文件类型
        );

    if (filePath.isEmpty()) {//用户取消选择
        return;
    }

    QFile file(filePath);
    //打开文件：只写+文本模式，不存在则创建，存在则覆盖
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误",
                             "TXT文件打开失败！\n原因：" + file.errorString());
        return;
    }

    QTextStream out(&file);
    int successCount = 0; //记录成功写入的学生数


    for (Student* stu : m_studentList) {//遍历本地学生列表，按格式写入
        //校验学生信息完整性，不完整则跳过
        if (!isStudentInfoComplete(stu)) {
            continue;
        }

        //提取学生信息，去除首尾空格，与读取格式一致
        QString name = stu->getname().trimmed();
        QString gender = stu->getgender().trimmed();
        QString id = stu->getid().trimmed();
        double score = stu->getscore();

        // 按姓名,性别,学号,成绩格式拼接，与读取格式一致
        QString stuLine = QString("%1,%2,%3,%4")
                              .arg(name)
                              .arg(gender)
                              .arg(id)
                              .arg(score);
        out << stuLine << "\n"; // 写入一行，用"\n"换行

        successCount++; //成功写入计数
    }
    file.close(); //关闭文件

    // 提示写入结果（仿照你的提示格式，风格统一）
    if (successCount > 0) {
        QMessageBox::information(this, "成功",
                                 QString("TXT数据写入完成！\n共写入%1名学生").arg(successCount));
    } else {
        QMessageBox::information(this, "提示", "未写入任何学生数据（无完整信息的学生）");
    }
}

void MainWindow::WriteDataToDB()//向选定db文件读取数据
{
    //弹出文件保存对话框，仅允许选择/创建 .db 文件
    QString dbPath = QFileDialog::getSaveFileName(
        this,
        "选择要写入的SQLite数据库",     //对话框标题
        QCoreApplication::applicationDirPath(), //初始路径：应用运行目录
        "SQLite数据库文件 (*.db)"       //仅显示/保存 .db 格式文件
        );

    if (dbPath.isEmpty()) { //用户取消选择，直接返回
        return;
    }

    //数据库连接
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbPath); //使用用户选择的 .db 文件路径
    }

    if (!db.open()) {// 打开数据库
        QMessageBox::warning(this, "数据库连接失败",
                             "无法连接到SQLite数据库！\n错误信息：" + db.lastError().text());
        return;
    }

    // 自动创建学生表
    QSqlQuery createQuery;
    QString createSql = R"(
        CREATE TABLE IF NOT EXISTS student_info (
            name VARCHAR(50) NOT NULL,
            gender VARCHAR(10) NOT NULL,
            id VARCHAR(20) NOT NULL UNIQUE,
            score DOUBLE NOT NULL,
            PRIMARY KEY (id)
        );
    )";
    if (!createQuery.exec(createSql)) {
        QMessageBox::warning(this, "建表失败",
                             "自动创建学生表出错！\n错误信息：" + createQuery.lastError().text());
        db.close();
        return;
    }

    //清空原有数据
    QSqlQuery clearQuery;
    if (!clearQuery.exec("DELETE FROM student_info;")) {
        QMessageBox::warning(this, "清空原有数据失败",
                             "无法清空数据库中原有学生数据！\n错误信息：" + clearQuery.lastError().text());
        db.close();
        return;
    }

    // 批量写入数据
    QSqlQuery insertQuery;
    int successCount = 0;
    QString errorMsg;

    QString insertSql = "INSERT INTO student_info (name, gender, id, score) VALUES (?, ?, ?, ?);";
    insertQuery.prepare(insertSql);

    for (Student* stu : m_studentList) {//判断数据合理性并写入
        if (!stu) {
            continue;
        }

        QString name = stu->getname().trimmed();
        QString gender = stu->getgender().trimmed();
        QString id = stu->getid().trimmed();
        double score = stu->getscore();

        if (name.isEmpty() || id.isEmpty()) {
            errorMsg += QString("学号「%1」：姓名或学号不能为空，跳过写入\n").arg(id);
            continue;
        }
        if (score < 0 || score > 100) {
            errorMsg += QString("学号「%1」：成绩%2无效，跳过写入\n").arg(id).arg(score);
            continue;
        }

        insertQuery.bindValue(0, name);
        insertQuery.bindValue(1, gender);
        insertQuery.bindValue(2, id);
        insertQuery.bindValue(3, score);

        if (!insertQuery.exec()) {
            errorMsg += QString("学号「%1」：插入失败，%2\n").arg(id).arg(insertQuery.lastError().text());
            continue;
        }

        successCount++;
        insertQuery.finish();
    }

    db.close();
    QString msg = QString("SQLite数据写入完成！\n成功写入%1名学生\n").arg(successCount);
    if (!errorMsg.isEmpty()) {
        msg += "失败信息：\n" + errorMsg;
    }
    QMessageBox::information(this, "写入结果", msg);
}

void MainWindow::on_search_button_clicked()//查找函数按钮
{
    QString keyword = ui->search_input->text().trimmed();

    if (!keyword.isEmpty() && m_isEditMode) {
        on_change_button_clicked(); //查询时禁止修改，自动调用权限修改按钮的槽函数，切到只读模式
    }
    else if(keyword.isEmpty()&& !m_isEditMode)
    {
        on_change_button_clicked();
    }
    //打印前先清空索引数组m_displayIndices
    m_displayIndices.clear();

    //情况1：关键词为空：调用showAllStudents（即自动填充所有学生序号）
    if (keyword.isEmpty()) {
        showAllStudents();
        return;
    }

    // 情况2：有关键词：遍历学生，做“模式匹配判断，记录序号
    for (size_t i = 0; i < m_studentList.size(); ++i) {
        Student* stu = m_studentList[i];
        if (!stu) continue; //空指针判断

        //检查学生所有信息中是否包含关键词
        QString name = stu->getname();
        QString gender = stu->getgender();
        QString id = stu->getid();
        QString score = QString::number(stu->getscore());
        QString grade = stu->getgrade();

        //只要有一个信息包含关键词，就记录该学生的序号i
        bool isMatch = name.contains(keyword)
                       || gender.contains(keyword)
                       || id.contains(keyword)
                       || score.contains(keyword)
                       || grade.contains(keyword);

        if (isMatch) {
            m_displayIndices.push_back(static_cast<int>(i)); // 只存序号，不做任何修改
        }
    }

    //刷新表格（只根据索引读取数据，不修改其他内容）
    refreshTable();

    //查询后表格设为只读，禁止修改
    ui->stu_info->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //若不存在，表格为空，输出提示
    if (m_displayIndices.empty()) {
        QMessageBox::information(this, "搜索结果", "未找到包含关键词的学生");
    }
}

void MainWindow::on_stu_info_itemChanged(QTableWidgetItem* item)//添加学生辅助函数
{
    if (!m_isEditMode) return; //只读模式不处理

    int row = item->row();
    int col = item->column();

    //等级列不可改，直接返回
    if (col == 4) {
        // ---------------- 修复空指针访问（核心修改2） ----------------
        // 先检查等级列Item是否存在，不存在则重新创建
        QTableWidgetItem* gradeItem = ui->stu_info->item(row, 4);
        if (gradeItem == nullptr) {
            gradeItem = new QTableWidgetItem("F");
            gradeItem->setFlags(gradeItem->flags() & ~Qt::ItemIsEditable);
            ui->stu_info->setItem(row, 4, gradeItem); // 重新绑定到表格
        }
        // 再设置等级文本（此时gradeItem一定有效）
        gradeItem->setText(m_studentList[row]->getgrade());
        return;
    }

    //其他列逻辑
    Student* stu = m_studentList[row];
    switch (col) {
    case 0: stu->setname(item->text()); break;
    case 1: stu->setgender(item->text()); break;
    case 2: { // 学号列：新增行（旧值为空）时跳过校验
        QString oldValue = stu->getid();
        QString newId = item->text().trimmed();

        //如果旧学号为空（新增行），只赋值，不校验
        if (oldValue.isEmpty()) {
            stu->setid(newId); //直接赋值，不判断空/重复
            return; //跳过后续校验
        }

        //旧学号非空（编辑已有学生）才做非空和重复校验
        if (newId.isEmpty()) {
            QMessageBox::warning(this, "错误", "学号不能为空！");
            item->setText(oldValue); //恢复原值
            return;
        }
        if (isIdExists(newId, row)) { //排除当前行自身
            QMessageBox::warning(this, "错误", "学号" + newId + "已存在！");
            item->setText(oldValue); //恢复原值
            return;
        }
        stu->setid(item->text());
        break;
    }
    case 3: {
        bool isNum;
        double score = item->text().toDouble(&isNum);
        if (!isNum) {
            //处理非数字的错误提示
            QMessageBox::warning(this, "提示", "成绩必须是0-100的数字！");
            item->setText(QString::number(stu->getscore()));
        } else if (!stu->setscore(score)) {
            //setscore已处理范围错误并提示，此处仅重置文本无需再次提示
            item->setText(QString::number(stu->getscore()));
        }
        //更新等级列
        QTableWidgetItem* gradeItem = ui->stu_info->item(row, 4);
        if (gradeItem == nullptr) {
            gradeItem = new QTableWidgetItem("F");
            gradeItem->setFlags(gradeItem->flags() & ~Qt::ItemIsEditable);
            ui->stu_info->setItem(row, 4, gradeItem);
        }
        gradeItem->setText(stu->getgrade());
        break;
        }
    }
    calculate_statistic();//更新统计页数据
}

void MainWindow::calculate_statistic()//统计页数据计算函数
{
    //初始化统计变量
    int totalCount = m_studentList.size();
    int countA = 0, countB = 0, countC = 0, countD = 0, countF = 0;

    //遍历学生列表统计等级
    for (Student* stu : m_studentList) {
        if (!stu) continue;  //空指针防御
        QString grade = stu->getgrade();
        if (grade == "A") countA++;
        else if (grade == "B") countB++;
        else if (grade == "C") countC++;
        else if (grade == "D") countD++;
        else if (grade == "F") countF++;
    }

    //计算百分比（处理总人数为0的边界条件）
    double rateA = 0.0, rateB = 0.0, rateC = 0.0, rateD = 0.0, rateF = 0.0;
    double passRate = 0.0;       // 及格（A-D）比例
    double excellentRate = 0.0;  // 优良（A-B）比例

    if (totalCount > 0) {
        rateA = (countA * 100.0) / totalCount;
        rateB = (countB * 100.0) / totalCount;
        rateC = (countC * 100.0) / totalCount;
        rateD = (countD * 100.0) / totalCount;
        rateF = (countF * 100.0) / totalCount;

        passRate = (countA + countB + countC + countD) * 100.0 / totalCount;
        excellentRate = (countA + countB) * 100.0 / totalCount;
    }

    //更新统计页面数据（确保统计页面实例存在）
    if (!m_statPage) {
        m_statPage = new statistic_page(this);  //初始化统计页面
    }

    m_statPage->setTotalCount(totalCount);
    m_statPage->setPassRate(passRate);
    m_statPage->setExcellentRate(excellentRate);
    m_statPage->setGradeCount("A", countA, rateA);
    m_statPage->setGradeCount("B", countB, rateB);
    m_statPage->setGradeCount("C", countC, rateC);
    m_statPage->setGradeCount("D", countD, rateD);
    m_statPage->setGradeCount("F", countF, rateF);
}

bool MainWindow::isIdExists(const QString& id, int currentRow)//辅助函数，判断学号不重复
{
    for (size_t i = 0; i < m_studentList.size(); ++i) {
        //排除当前行（编辑自身时不与自己冲突）
        if (currentRow != -1 && static_cast<int>(i) == currentRow) {
            continue;
        }
        if (m_studentList[i]->getid() == id) {
            return true; // 存在重复
        }
    }
    return false;
}

bool MainWindow::isStudentInfoComplete(Student* stu)//辅助函数判断学生信息完整性
{
    if (!stu) { //学生对象为空，直接返回false
        return false;
    }

    //获取学生信息并去除前后空格
    QString name = stu->getname().trimmed();
    QString gender = stu->getgender().trimmed();
    QString id = stu->getid().trimmed();
    double score = stu->getscore();

    //完整性判断条件（与读取时一致，确保数据有效）
    //1. 姓名、性别、学号不能为空
    if (name.isEmpty() || gender.isEmpty() || id.isEmpty()) {
        return false;
    }
    //2. 成绩必须在0-100之间
    if (score < 0 || score > 100) {
        return false;
    }

    return true; // 信息完整，可写入
}
