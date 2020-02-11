#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QGraphicsScene>
#include <memory>

namespace Ui {
class MainWindow;
}

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

private:
    void setup_gs();

    void convert_rgb_to_bw(QImage& input_image);
};

#endif // MAINWINDOW_HPP
