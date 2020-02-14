#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QGraphicsScene>
#include <memory>

namespace Ui {
class MainWindow;
}

class LRTThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_m_push_button_load_image_clicked();

    void on_m_push_button_lrt_clicked();

    void on_m_push_button_export_image_clicked();

private:
    Ui::MainWindow* ui;
    std::shared_ptr<QGraphicsScene>  m_screen{nullptr};
    QString m_path_save;

    std::shared_ptr<QImage> m_image;

    std::shared_ptr<LRTThread> m_lrt_thread;

private:
    void setup_gs();

    void convert_rgb_to_bw(std::shared_ptr<QImage>& input_image);
};

#endif // MAINWINDOW_HPP
