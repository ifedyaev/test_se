#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP
/***********************************
 *    GIU Line Radon Transform
 ***********************************
 */

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

    void end_of_job_radon(const std::shared_ptr<QImage> data);

private:
    Ui::MainWindow* ui;

    QString m_path_save; /* save path open file dialog */

    bool m_is_compute{false};

    std::shared_ptr<QGraphicsScene> m_screen{nullptr}; /* gs -> gw */

    std::shared_ptr<QImage> m_image{nullptr};           /* input image */
    std::shared_ptr<QImage> m_image_output{nullptr};    /* output LRT imag */

    std::shared_ptr<LRTThread> m_lrt_thread{nullptr};   /* Line Radon Transform Thread */

private:
    /**
     * @brief setup_gs Graphics Scene
     */
    void setup_gs();

    /**
     * @brief convert_rgb_to_bw Conver input Image -> to white-black Image
     * @param input_image [input|output] - image convert
     */
    void convert_rgb_to_bw(std::shared_ptr<QImage>& input_image);
};

#endif // MAINWINDOW_HPP
