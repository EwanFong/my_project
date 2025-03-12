#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "image_processor.hpp"
#include "image_denoisy.hpp"
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QListWidgetItem>
#include <QPixmap>
#include <QMessageBox>
#include <QDebug>
#include <QShortcut>
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QDebug>
#include "Image_utils.hpp"
#include <QPushButton>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>
#include "Onnx_Inference_Wrapper.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scence(new QGraphicsScene (this))
    , processor(new ImageProcessor(cv::Mat()))
    , denoiser(nullptr)
{
    drawkeymap_ = {
        {Qt::Key_1, ImageProcessor::DRAW_FREEHAND},
        {Qt::Key_2, ImageProcessor::DRAW_RECTANGLE},
        {Qt::Key_3, ImageProcessor::DRAW_CIRCLE},
        {Qt::Key_4, ImageProcessor::DRAW_TRIANGLE},
        {Qt::Key_5, ImageProcessor::DRAW_MOSAIC}
    };

    denoisykeymap_ = {
        {Qt::Key_1, ImageProcessor_denoisy::DENOISY_BLUR},
        {Qt::Key_2, ImageProcessor_denoisy::DENOISY_MEDIANBLUR},
        {Qt::Key_3, ImageProcessor_denoisy::DENOISY_GAUSSIANBLUR},
        {Qt::Key_4, ImageProcessor_denoisy::DENOISY_BILATERALFILTER}
    };

    ui->setupUi(this);
    ui->graphicsView->setScene(scence);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->setMouseTracking(true);

    processor = new ImageProcessor(cv::Mat(720, 1280, CV_8UC3, cv::Scalar(255,255,255)));

    connect(ui->btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenButtonClicked);
    connect(ui->listFileBrowser, &QListWidget::itemClicked, this, &MainWindow::onFileItemClicked);
    connect(ui->btnDraw, &QPushButton::clicked, this, &MainWindow::onDrawButtonClicked);
    connect(ui->btnDenoise, &QPushButton::clicked, this, &MainWindow::onDenoiseButtonClicked);
    connect(ui->btnInpaint, &QPushButton::clicked, this, &MainWindow::onInpaintButtonClicked);

    setupModeShortcuts();
    setupEssentialShortcuts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->graphicsView->viewport()){
        if (event->type() != QEvent::MouseButtonPress &&
            event->type() != QEvent::MouseMove &&
            event->type() != QEvent::MouseButtonRelease){
            return false;
        }

        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (!mouseEvent) return false;
        if (!processor) {
            qWarning() << "ImageProcessor is null";
            return false;
        }

        const QPoint qtPos = mouseEvent->pos();

        QPointF scenePos = ui->graphicsView->mapToScene(qtPos);
        cv::Point cvPos(scenePos.x(), scenePos.y());

        int flags = 0;
        if (mouseEvent->buttons() & Qt::LeftButton) flags |= cv::EVENT_FLAG_LBUTTON;
        if (mouseEvent->buttons() & Qt::RightButton) flags |= cv::EVENT_FLAG_RBUTTON;

        if (convertQtEventType(event->type()) != -1){
            ImageProcessor::onMouse(
                convertQtEventType(event->type()),
                cvPos.x, cvPos.y,
                flags,
                processor);

            QMetaObject::invokeMethod(this, [this](){
                updateGraphicsView();
            }, Qt::QueuedConnection);

            if (currentActiveMode_ == MODE_DRAW){
                if (event->type() == QEvent::MouseButtonRelease){
                pushHistory();
                qDebug() << "the number of historystack:" << historyStack.size();
                }
            }
        }
        return true;
        qDebug() << "transformed event kind:" << convertQtEventType(event->type())
                 << "location:" << cvPos.x << cvPos.y;
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::pushHistory()
{
    std::lock_guard<std::mutex> lock(processor->mutex_);
    if (historyStack.size() >= Max_HISTORY){
        historyStack.erase(MainWindow::historyStack.begin());
    }
    historyStack.push_back(processor->original_.clone());
}

int MainWindow::convertQtEventType(QEvent::Type type){
        switch (type) {
        case QEvent::MouseButtonPress:
            return cv::EVENT_LBUTTONDOWN;
        case QEvent::MouseMove:
            return cv::EVENT_MOUSEMOVE;
        case QEvent::MouseButtonRelease:
            return cv::EVENT_LBUTTONUP;
        default:
            return -1;
        }
}

void MainWindow::updateGraphicsView()
{
    if (processor->working_.empty()){
        qWarning() << "working image is empty";
        return;
    }

    cv::Mat displayMat;
    {
        std::lock_guard<std::mutex> lock(processor->mutex_);
        cv::cvtColor(processor->working_, displayMat, cv::COLOR_BGR2RGB);
    }
    QImage img(displayMat.data,
               displayMat.cols,
               displayMat.rows,
               displayMat.step,
               QImage::Format_RGB888);
        img = img.copy();

    scence->clear();
    scence->addPixmap(QPixmap::fromImage(img));

    scence->setSceneRect(img.rect());
    ui->graphicsView->fitInView(scence->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsView->viewport()->update();
}



void MainWindow::onOpenButtonClicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("choose the folder"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (folderPath.isEmpty()) return;

    QDir dir(folderPath);
    QStringList filters = {"*.png", "*.jpg"};
    imageFiles = dir.entryList(filters, QDir::Files);

    if(imageFiles.isEmpty()){
        QMessageBox::warning(this,"warning" ,"no files in folder");
        return;
    }

    currentFolder = folderPath;
    ui->listFileBrowser->clear();
    foreach (const QString &file, imageFiles) {
        ui->listFileBrowser->addItem(file);
    }

    if(!imageFiles.isEmpty())
    {
        loadImage(dir.filePath(imageFiles.first()));
    }
}

void MainWindow::onFileItemClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString fileName = item->text();
    QString filePath = QDir(currentFolder).filePath(fileName);
    loadImage(filePath);
}

void MainWindow::loadImage(const QString &filePath)
{
    clearView();

    currentFilePath = filePath;

    QPixmap pixmap(filePath);
    if(pixmap.isNull()) {
        QMessageBox::warning(this, "Error", "cannot load the file");
        return;
    }

    cv::Mat newImage = ImageUtils::QImageToMat(pixmap.toImage());

    {
        std::lock_guard<std::mutex> lock(processor->mutex_);
        processor->original_ = newImage;
        processor->working_ = newImage.clone();
    }

    scence->addPixmap(pixmap);
    scence->setSceneRect(pixmap.rect());
    ui->graphicsView->fitInView(scence->sceneRect(), Qt::KeepAspectRatio);

    historyStack.clear();
    pushHistory();
}

void MainWindow::onInpaintButtonClicked()
{
    setEnabled(false);
    qDebug() << "inferencing....";

    QCoreApplication::processEvents();

    try{
        cv::Mat input_img;
        {
            std::lock_guard<std::mutex> lock(processor->mutex_);
            input_img = processor->working_.clone();
        }

        ONNXInferenceWrapper upscaler("models/RealESRGAN_x4plus.onnx");
        cv::Mat processed = upscaler.pre_downScale(input_img);
        cv::Mat output = upscaler.enhance(processed);
        cv::Mat fin_result = upscaler.fin_upScale(output);

        {
            std::lock_guard<std::mutex> lock(processor->mutex_);
            processor->original_ = fin_result.clone();
            processor->working_ = fin_result.clone();
        }

        pushHistory();
        updateGraphicsView();

        qDebug() << "inference success";
    }catch(const std::exception& e){
        qDebug() << "inference fail" << e.what();
    }
    setEnabled(true);
}

void MainWindow::handleFileSave()
{
    if (currentFilePath.isEmpty()){
        QMessageBox::warning(this, "save failed" ,"nothing can be saved");
        return;
    }

    QString saveDir = QFileDialog::getExistingDirectory(this, "please choose the save dir",
                                                        QDir::homePath(),
                                                        QFileDialog::ShowDirsOnly);

    if (saveDir.isEmpty()) return;

    QFileInfo fileInfo(currentFilePath);
    QString savepath = QDir(saveDir).filePath(fileInfo.fileName());

    std::lock_guard<std::mutex> lock(processor->mutex_);
    try {
        if (!cv::imwrite(savepath.toStdString(), processor->original_)){
            throw std::runtime_error("fail to save");
        }
        QMessageBox::information(this, "save successfully",
                                 QString("file is saved to:\n%1").arg(savepath));
    } catch (const std::exception& e){
        QMessageBox::critical(this, "save error",
                              QString("error message: %1").arg(e.what()));
    }
}

void MainWindow::clearView()
{
    scence->clear();
    ui->graphicsView->resetTransform();
}

void MainWindow::decreaseSize()
{
    adjustSize(false);
}

void MainWindow::increaseSize()
{
    adjustSize(true);
}

void MainWindow::adjustSize(bool isIncrease)
{
    std::lock_guard<std::mutex> lock(processor->mutex_);

    if (currentActiveMode_ == MODE_DENOISE){
        const int kernel_step = 2;
        int newSize = denoiseKernelSize_ + (isIncrease ? kernel_step : -kernel_step);
        newSize = std::clamp(newSize, 3, 15);
        denoiseKernelSize_ = newSize;
        qDebug() << "[Denoise] Kernel size:" << denoiseKernelSize_;
    }
    else if (currentActiveMode_ == MODE_DRAW){
        switch (processor->currentMode_) {
            case ImageProcessor::DRAW_FREEHAND:
                processor->brushSize = std::clamp
                    (processor->brushSize + (isIncrease ? 1 : -1), 1, 6);
                qDebug() << "brushSize is:" << processor->brushSize;
                break;

            case ImageProcessor::DRAW_RECTANGLE:
                processor->Border_Width = std::clamp
                    (processor->Border_Width + (isIncrease ? 1 : -1), 3, 6);
                qDebug() << "Border width is:" << processor->Border_Width;
                break;
            case ImageProcessor::DRAW_CIRCLE:
                processor->Border_Width = std::clamp
                    (processor->Border_Width + (isIncrease ? 1 : -1), 3, 6);
                qDebug() << "Border width is:" << processor->Border_Width;
                break;
            case ImageProcessor::DRAW_TRIANGLE:
                processor->Border_Width = std::clamp
                    (processor->Border_Width + (isIncrease ? 1 : -1), 3, 6);
                qDebug() << "Border width is:" << processor->Border_Width;
                break;
            case ImageProcessor::DRAW_MOSAIC:
                processor->mosaicSize = std::clamp
                    (processor->mosaicSize + (isIncrease ? 2 : -2), 5, 11);
                qDebug() << "mosaicSize is:" << processor->mosaicSize;
                break;

            default:
                break;
        }
    }
}

void MainWindow::setupModeShortcuts()
{
    QList<int> allkeys = {Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4, Qt::Key_5};

    foreach (int key, allkeys) {
        QShortcut *shortcut = new QShortcut(QKeySequence(key), this);
        connect(shortcut, &QShortcut::activated, this, [this, key](){handleShortcuts(key);});
    }
}

void MainWindow::setupEssentialShortcuts()
{
    new QShortcut(QKeySequence(Qt::Key_Q), this, [this](){ adjustSize(false);});
    new QShortcut(QKeySequence(Qt::Key_E), this, [this](){ adjustSize(true);});
    new QShortcut(QKeySequence::Save, this, SLOT(handleFileSave()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this, SLOT(undoLastOperation()));
}

void MainWindow::handleShortcuts(int key)
{
    switch (currentActiveMode_) {
    case MODE_DRAW:
        if (drawkeymap_.contains(key)){
            currentDrawMode_ = drawkeymap_[key];
            processor->setMode(currentDrawMode_);
            qDebug() << "Draw tools change to:" << ImageProcessor::toolsToString(currentDrawMode_);
        }
        break;

    case MODE_DENOISE:
        if (denoisykeymap_.contains(key)){
            currentDenoiseMode_ = denoisykeymap_[key];
            applyDenoise();
            qDebug() << "Denoise method:" << ImageProcessor_denoisy::methodToString(currentDenoiseMode_)
                     << "Kernel Size:" << denoiseKernelSize_;
        }
    default:
        break;
    }
}

void MainWindow::applyDenoise()
{
    if(processor->original_.empty()) return;

    denoiser = new ImageProcessor_denoisy(processor->original_);
    cv::Mat denoised = denoiser->denoiseImage(currentDenoiseMode_ ,denoiseKernelSize_);

    {
        std::lock_guard<std::mutex> lock(processor->mutex_);
        processor->original_ = denoised.clone();
        processor->working_ = denoised.clone();
    }

    pushHistory();
    qDebug() << "the number of historystack:" << historyStack.size();
    updateGraphicsView();
}

void MainWindow::onDrawButtonClicked(bool checked)
{
    currentActiveMode_ = checked ? MODE_DRAW : MODE_NONE;
    ui->btnDraw->setChecked(checked);
    if (checked){
        ui->btnDenoise->setChecked(false);
        currentDenoiseMode_ = ImageProcessor_denoisy::DENOISY_NONE;
        currentDrawMode_ = ImageProcessor::DRAW_NONE;
        qDebug() << "enter mode DRAW, please use key 1~5 to change tools";
    }else{
        currentDrawMode_ = ImageProcessor::DRAW_NONE;
        processor->setMode(ImageProcessor::DRAW_NONE);
    }
}

void MainWindow::onDenoiseButtonClicked(bool checked)
{
    currentActiveMode_ = checked ? MODE_DENOISE : MODE_NONE;
    ui->btnDenoise->setChecked(checked);
    if (checked){
        ui->btnDraw->setChecked(false);
        currentDrawMode_ = ImageProcessor::DRAW_NONE;
        processor->setMode(ImageProcessor::DRAW_NONE);
        currentDenoiseMode_ = ImageProcessor_denoisy::DENOISY_NONE;
        qDebug() << "enter mode DENOISE, please use key 1~4 to change denoise-ways";
    }
}

void MainWindow::loadInferenceResult(const QString& path)
{
    cv::Mat result = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
    if (result.empty()) {
        QMessageBox::critical(this, "Error", "Failed to load result image");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(processor->mutex_);
        processor->original_ = result.clone();
        processor->working_ = result.clone();
    }

    updateGraphicsView();
}

void MainWindow::updateProcessorMode(ImageProcessor::ProcessingMode mode)
{
    processor->setMode(mode);
    updateGraphicsView();
    qDebug() << "change to" << mode;
}

void MainWindow::undoLastOperation()
{
    if(historyStack.size() > 1){
        {
        std::lock_guard<std::mutex> lock(processor->mutex_);
        historyStack.pop_back();

        }
        cv::Mat prevState = historyStack.back().clone();
        processor->original_ = prevState.clone();
        processor->working_ = prevState.clone();

        updateGraphicsView();
        qDebug() << "[Undo] Restored to history index:" << historyStack.size() - 1;
    }else{
        qWarning() << "cannot undo further";
        return;
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    switch(event->key()) {
    case Qt::Key_A:
        qDebug() << "A key pressed";
        break;
    case Qt::Key_Escape:
        close();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}
