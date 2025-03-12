/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *mainLayout;
    QSplitter *mainSplitter;
    QWidget *leftPanel;
    QVBoxLayout *leftLayout;
    QHBoxLayout *toolbarLayout;
    QSpacerItem *horizontalSpacerLeft;
    QPushButton *btnOpen;
    QPushButton *btnDraw;
    QPushButton *btnDenoise;
    QPushButton *btnInpaint;
    QSpacerItem *horizontalSpacerRight;
    QListWidget *listFileBrowser;
    QGraphicsView *graphicsView;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1280, 720);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QHBoxLayout(centralwidget);
        mainLayout->setObjectName("mainLayout");
        mainSplitter = new QSplitter(centralwidget);
        mainSplitter->setObjectName("mainSplitter");
        mainSplitter->setOrientation(Qt::Horizontal);
        mainSplitter->setHandleWidth(6);
        leftPanel = new QWidget(mainSplitter);
        leftPanel->setObjectName("leftPanel");
        leftLayout = new QVBoxLayout(leftPanel);
        leftLayout->setObjectName("leftLayout");
        leftLayout->setContentsMargins(0, 0, 0, 0);
        toolbarLayout = new QHBoxLayout();
        toolbarLayout->setObjectName("toolbarLayout");
        horizontalSpacerLeft = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        toolbarLayout->addItem(horizontalSpacerLeft);

        btnOpen = new QPushButton(leftPanel);
        btnOpen->setObjectName("btnOpen");
        btnOpen->setMinimumSize(QSize(100, 40));
        btnOpen->setStyleSheet(QString::fromUtf8("\n"
"              QPushButton {\n"
"                background:rgb(209, 165, 77);\n"
"                border-radius: 4px;\n"
"                color: white;\n"
"              }\n"
"              QPushButton:hover { background:rgb(8, 175, 247); }\n"
"             "));

        toolbarLayout->addWidget(btnOpen);

        btnDraw = new QPushButton(leftPanel);
        btnDraw->setObjectName("btnDraw");
        btnDraw->setCheckable(true);
        btnDraw->setStyleSheet(QString::fromUtf8("\n"
"              QPushButton:hover { background:rgb(205, 7, 175); }\n"
"              QPushButton:checked {    \n"
"                background:rgb(205, 7, 175);  \n"
"                color: white;\n"
"                border: none;\n"
"              }\n"
"            "));
        btnDraw->setMinimumSize(QSize(100, 40));

        toolbarLayout->addWidget(btnDraw);

        btnDenoise = new QPushButton(leftPanel);
        btnDenoise->setObjectName("btnDenoise");
        btnDenoise->setCheckable(true);
        btnDenoise->setStyleSheet(QString::fromUtf8("\n"
"            QPushButton:hover { background:rgb(91, 8, 247); }\n"
"            QPushButton:checked {      \n"
"                background:rgb(91, 8, 247);    \n"
"                color: white;\n"
"                border: none;\n"
"            }\n"
"            "));
        btnDenoise->setMinimumSize(QSize(100, 40));

        toolbarLayout->addWidget(btnDenoise);

        btnInpaint = new QPushButton(leftPanel);
        btnInpaint->setObjectName("btnInpaint");
        btnInpaint->setStyleSheet(QString::fromUtf8("\n"
"              QPushButton:hover { background:rgb(214, 53, 13); }\n"
"             "));
        btnInpaint->setMinimumSize(QSize(100, 40));

        toolbarLayout->addWidget(btnInpaint);

        horizontalSpacerRight = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        toolbarLayout->addItem(horizontalSpacerRight);


        leftLayout->addLayout(toolbarLayout);

        listFileBrowser = new QListWidget(leftPanel);
        listFileBrowser->setObjectName("listFileBrowser");
        listFileBrowser->setMinimumSize(QSize(300, 200));
        listFileBrowser->setStyleSheet(QString::fromUtf8("\n"
"            QListWidget {\n"
"                background: white;\n"
"                color: black;\n"
"                border: 1px solid #cccccc;\n"
"                font-family: Segoe UI;\n"
"                font-size: 12px;\n"
"            }\n"
"            QListWidget::item {\n"
"                height: 24px;\n"
"                border-bottom: 1px solid #eeeeee;\n"
"            }\n"
"            QListWidget::item:hover {\n"
"                background: #f0f6fc;\n"
"                color: black;\n"
"            }\n"
"            QListWidget::item:selected {\n"
"                background: #0078d4;\n"
"                color: white;\n"
"            }\n"
"           "));
        listFileBrowser->setAlternatingRowColors(false);

        leftLayout->addWidget(listFileBrowser);

        mainSplitter->addWidget(leftPanel);
        graphicsView = new QGraphicsView(mainSplitter);
        graphicsView->setObjectName("graphicsView");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(3);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(graphicsView->sizePolicy().hasHeightForWidth());
        graphicsView->setSizePolicy(sizePolicy);
        graphicsView->setStyleSheet(QString::fromUtf8("\n"
"         background: #f5f5f5;\n"
"         border: 1px solid #cccccc;\n"
"         border-radius: 3px;\n"
"        "));
        mainSplitter->addWidget(graphicsView);

        mainLayout->addWidget(mainSplitter);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Image Processor", nullptr));
        btnOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
        btnDraw->setText(QCoreApplication::translate("MainWindow", "Draw (2-5/Z)", nullptr));
#if QT_CONFIG(tooltip)
        btnDraw->setToolTip(QCoreApplication::translate("MainWindow", "DrawingMode - shortcut: 1freehand 2rectangle 3triangle 4circle 5moscia Zundo", nullptr));
#endif // QT_CONFIG(tooltip)
        btnDraw->setText(QCoreApplication::translate("MainWindow", "Draw", nullptr));
        btnDenoise->setText(QCoreApplication::translate("MainWindow", "Denoise", nullptr));
        btnInpaint->setText(QCoreApplication::translate("MainWindow", "Inpaint", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
