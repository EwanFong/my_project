#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QListWidget>
#include <vector>
#include "image_processor.hpp"
#include "image_denoisy.hpp"
#include <QEvent>
#include <QDebug>
#include "Onnx_Inference_Wrapper.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ActiveMode {
        MODE_NONE,
        MODE_DRAW,
        MODE_DENOISE
    };
    using DrawMode = ImageProcessor::ProcessingMode;
    using DenoiseMode = ImageProcessor_denoisy::denoisyMode;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onOpenButtonClicked();
    void onFileItemClicked(QListWidgetItem * item);
    void onDrawButtonClicked(bool checked);
    void onDenoiseButtonClicked(bool checked);
    void onInpaintButtonClicked();
    void undoLastOperation();
    void handleFileSave();

private:
    using DrawKeyMap = QMap<int, DrawMode>;
    using DenoisyKeyMap = QMap<int, DenoiseMode>;

    Ui::MainWindow *ui;
    QGraphicsScene *scence;
    QString currentFolder;
    QString currentFilePath;
    QStringList imageFiles;
    std::vector<cv::Mat> historyStack;
    ImageProcessor *processor;
    ImageProcessor_denoisy *denoiser;

    const int Max_HISTORY = 15;
    int denoiseKernelSize_ = 5;

    bool isDrawModeActive_ = false;
    bool isDenoiseModeActive_ = false;
    DrawMode currentDrawMode_ = ImageProcessor::DRAW_NONE;
    DenoiseMode currentDenoiseMode_ = ImageProcessor_denoisy::DENOISY_NONE;
    ActiveMode currentActiveMode_ = MODE_NONE;
    DrawKeyMap drawkeymap_;
    DenoisyKeyMap denoisykeymap_;

    void loadImage(const QString &filePath);
    void loadInferenceResult(const QString& path);
    void clearView();
    void updateProcessorMode(ImageProcessor::ProcessingMode mode);
    void setupModeShortcuts();
    void setupEssentialShortcuts();
    void handleShortcuts(int key);
    void applyDenoise();
    void updateGraphicsView();
    void decreaseSize();
    void increaseSize();
    void adjustSize(bool isIncrease);
    void pushHistory();
    int convertQtEventType(QEvent::Type type);
};

