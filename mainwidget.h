#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "global.h"

#include <QMessageBox>
#include <QVariant>
#include <QScreen>
#include <QTimer>

#include "touchscreen.h"
#include "configwindow.h"
#include "gpconfigurator.h"


class Widget : public QWidget
{

private slots:
    void ShowContextMenu(const QPoint &pos);
private:
    QVBoxLayout  *layout;
    QFormLayout  *formLayout;
    QPushButton  *homeButton, *powerButton, *longPowerButton, *settingsConfigButton,
                 *clearImageButton, *configGamepadButton;
    QLineEdit    *addrLineEdit;
    QSlider      *touchOpacitySlider;
    QLabel       *creatorLabel, *titleLabel, *instructionsLabel;
    ConfigWindow *settingsConfig;
    QPixmap      *logo, *title;
    QScreen      *screen;
    QFrame       *lineA, *lineB, *lineC;
    QLabel       *monitorLabel;
    QFrame *lineB2()
    {
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setFrameShadow(QFrame::Sunken);
        return f;
    }
    QString       lastButtonEvent = "-", lastAxisEvent = "-";

    // ---- Monitor de entrada (diagnostico) ----
    static QString buttonName(int b)
    {
        switch (b) {
        case QGamepadManager::ButtonA: return "A";
        case QGamepadManager::ButtonB: return "B";
        case QGamepadManager::ButtonX: return "X";
        case QGamepadManager::ButtonY: return "Y";
        case QGamepadManager::ButtonL1: return "L1";
        case QGamepadManager::ButtonR1: return "R1";
        case QGamepadManager::ButtonL2: return "L2";
        case QGamepadManager::ButtonR2: return "R2";
        case QGamepadManager::ButtonSelect: return "Select";
        case QGamepadManager::ButtonStart: return "Start";
        case QGamepadManager::ButtonL3: return "L3";
        case QGamepadManager::ButtonR3: return "R3";
        case QGamepadManager::ButtonUp: return "Up";
        case QGamepadManager::ButtonDown: return "Down";
        case QGamepadManager::ButtonRight: return "Right";
        case QGamepadManager::ButtonLeft: return "Left";
        case QGamepadManager::ButtonCenter: return "Center";
        case QGamepadManager::ButtonGuide: return "Guide";
        default: return QString("?%1").arg(b);
        }
    }

    static QString axisName(int a)
    {
        switch (a) {
        case QGamepadManager::AxisLeftX: return "LeftX";
        case QGamepadManager::AxisLeftY: return "LeftY";
        case QGamepadManager::AxisRightX: return "RightX";
        case QGamepadManager::AxisRightY: return "RightY";
        default: return QString("?%1").arg(a);
        }
    }

    static bool isHeld(QGamepadManager::GamepadButton b)
    {
        return b >= 0 && (buttons & (1 << b));
    }

    QString monitorText() const
    {
        QStringList held, sent;
        for (int b = QGamepadManager::ButtonA; b <= QGamepadManager::ButtonGuide; b++)
            if (isHeld(static_cast<QGamepadManager::GamepadButton>(b)))
                held << buttonName(b);

        // Lo que realmente se envia a la 3DS (mismo orden que Worker::sendFrame)
        static const char *middle[8] = {"Select", "Start", "D-Right", "D-Left", "D-Up", "D-Down", "R", "L"};
        static const char *ab[2] = {"A", "B"};
        static const char *xy[2] = {"X", "Y"};
        static const char *ir[2] = {"ZR", "ZL"};
        for (int i = 0; i < 2; i++) if (isHeld(hidButtonsAB[i])) sent << ab[i];
        for (int i = 0; i < 8; i++) if (isHeld(hidButtonsMiddle[i])) sent << middle[i];
        for (int i = 0; i < 2; i++) if (isHeld(hidButtonsXY[i])) sent << xy[i];
        for (int i = 0; i < 2; i++) if (isHeld(irButtons[i])) sent << ir[i];

        MyAxis l = worker.getLeftAxis(), r = worker.getRightAxis();
        return QString("Input monitor\n"
                       "Last button: %1\n"
                       "Last axis: %2\n"
                       "Held (gamepad): %3\n"
                       "Sent to 3DS: %4\n"
                       "Circle Pad: x=%5 y=%6\n"
                       "C-Stick: x=%7 y=%8")
                .arg(lastButtonEvent, lastAxisEvent,
                     held.isEmpty() ? "-" : held.join(", "),
                     sent.isEmpty() ? "-" : sent.join(", "))
                .arg(l.x, 0, 'f', 2).arg(l.y, 0, 'f', 2)
                .arg(r.x, 0, 'f', 2).arg(r.y, 0, 'f', 2);
    }

public:
    TouchScreen *touchScreen;
    QString version = QCoreApplication::applicationVersion();

    Widget(QWidget *parent = nullptr) : QWidget(parent)
    {
        layout = new QVBoxLayout(this);

        addrLineEdit = new QLineEdit(this);
        addrLineEdit->setClearButtonEnabled(true);
        addrLineEdit->setInputMethodHints(Qt::ImhPreferNumbers);

        formLayout = new QFormLayout();
        formLayout->addRow(tr("IP address 📡"), addrLineEdit);

        touchOpacitySlider = new QSlider(Qt::Horizontal);
        touchOpacitySlider->setRange(0, 10);
        touchOpacitySlider->setValue(10);
        touchOpacitySlider->setTickInterval(1);
        //formLayout->addRow(tr("TS Opacity"), touchOpacitySlider);

        homeButton = new QPushButton(tr("Home 🏠"), this);
        homeButton->setFocusPolicy(Qt::NoFocus);
        powerButton = new QPushButton(tr("Power ⚫"), this);
        powerButton->setFocusPolicy(Qt::NoFocus);
        longPowerButton = new QPushButton(tr("Power (long) ⚫⚫⚫"), this);
        longPowerButton->setFocusPolicy(Qt::NoFocus);
        settingsConfigButton = new QPushButton(tr("Settings ⚙️"), this);
        settingsConfigButton->setFocusPolicy(Qt::NoFocus);
        //clearImageButton = new QPushButton(tr("Clear Image"), this);
        configGamepadButton = new QPushButton(tr("Configure Custom Gamepad 🎮"));
        configGamepadButton->setFocusPolicy(Qt::NoFocus);

        QScreen *screen = QApplication::screens().at(0);
        titleLabel = new  QLabel;
        titleLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
        QPixmap title(":/myressources/logo/InputRedirectionClient.png");
        titleLabel->setPixmap(title.scaledToWidth(screen->availableGeometry().width()-20));
        titleLabel->setAlignment(Qt::AlignCenter);

        lineA = new QFrame;
        lineA->setFrameShape(QFrame::HLine);
        lineA->setFrameShadow(QFrame::Sunken);

        lineB = new QFrame;
        lineB->setFrameShape(QFrame::HLine);
        lineB->setFrameShadow(QFrame::Sunken);

        lineC = new QFrame;
        lineC->setFrameShape(QFrame::HLine);
        lineC->setFrameShadow(QFrame::Sunken);

        instructionsLabel = new QLabel;
        instructionsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        instructionsLabel->setAlignment(Qt::AlignLeft);
        instructionsLabel->setText("<html><b>Instructions:</b>"
                                   "<br>1. Start InputRedirection on your 3DS via Rosalina menu (Press L+⬇️+Select)."
                                   "<br>2. Connect your Gamepad to your Android device.</br>"
                                   "<br>3. Enter the IP address displayed on your 3DS in the IRC application.</br>"
                                   "<br>ℹ️ Work via Wi-Fi or via Mobile Data if both devices share the same network.</br></html>");
        instructionsLabel->setTextFormat(Qt::RichText);
        instructionsLabel->setWordWrap(true);

        monitorLabel = new QLabel;
        monitorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        monitorLabel->setAlignment(Qt::AlignLeft);
        monitorLabel->setTextFormat(Qt::PlainText);
        monitorLabel->setWordWrap(true);
        monitorLabel->setStyleSheet("font-family: monospace; font-size: 10pt;");
        monitorLabel->setText("Input monitor\nMove a stick or press a button");

        creatorLabel = new QLabel;
        creatorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        creatorLabel->setAlignment(Qt::AlignCenter);
        creatorLabel->setText("<html>Created by <a href=\"https://github.com/TuxSH/\">TuxSH</a>"
                              "<br>Based on <a href=\"https://github.com/mastermune/\">mastermune</a> & <a href=\"https://github.com/gbrown5/\">gbrown5</a>'s forks"
                              "<br>Version "+version+" for Android by JambonBeurreMan</html>");
        creatorLabel->setTextFormat(Qt::RichText);
        creatorLabel->setOpenExternalLinks(true);

        if(screen->availableGeometry().height()<=800)
        {
            instructionsLabel->setStyleSheet("font: 10pt;");
            creatorLabel->setStyleSheet("font: 10pt");
        }

        setContextMenuPolicy(Qt::CustomContextMenu);

        // Disable/hide the configurator button if running windows since it's not supported
         if (QSysInfo::productType() == "windows" ||
             QSysInfo::productType() == "osx")
         {
             configGamepadButton->setEnabled(false);
             configGamepadButton->setVisible(false);
         }

        layout->setContentsMargins(10,10,10,0);
        layout->addWidget(titleLabel);
        layout->addSpacing(10);
        layout->addLayout(formLayout);
        layout->addWidget(lineA);
        layout->addWidget(homeButton);
        if(screen->availableGeometry().height()>=1280)
        {
            layout->addSpacing(50);
        }
        layout->addWidget(powerButton);
        layout->addWidget(longPowerButton);
        layout->addWidget(configGamepadButton);
        layout->addWidget(settingsConfigButton);
        //layout->addWidget(clearImageButton);
        layout->addWidget(lineB);
        layout->addWidget(instructionsLabel);
        layout->addWidget(lineB2());
        layout->addWidget(monitorLabel);
        layout->addStretch(1);
        layout->addWidget(lineC);
        layout->addWidget(creatorLabel);

        gpConfigurator = new GamepadConfigurator();

        // Monitor de entrada: ultimo evento recibido de cada tipo
        connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this,
                [this](int deviceId, QGamepadManager::GamepadButton button, double value)
        {
            lastButtonEvent = QString("PRESS %1 (dev %2, %3)").arg(buttonName(button)).arg(deviceId).arg(value, 0, 'f', 2);
        });
        connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this,
                [this](int deviceId, QGamepadManager::GamepadButton button)
        {
            lastButtonEvent = QString("RELEASE %1 (dev %2)").arg(buttonName(button)).arg(deviceId);
        });
        connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent, this,
                [this](int deviceId, QGamepadManager::GamepadAxis axis, double value)
        {
            lastAxisEvent = QString("%1 = %2 (dev %3)").arg(axisName(axis)).arg(value, 0, 'f', 2).arg(deviceId);
        });
        QTimer *monitorTimer = new QTimer(this);
        connect(monitorTimer, &QTimer::timeout, this, [this]() { monitorLabel->setText(monitorText()); });
        monitorTimer->start(100);

        connect(addrLineEdit, &QLineEdit::textChanged, this,
                [](const QString &text)
        {
            ipAddress = text;
            settings.setValue("ipAddress", text);
        });

        connect(configGamepadButton, &QPushButton::released, this,
                [](void)
        {
           gpConfigurator->showGui();
         });

        connect(homeButton, &QPushButton::pressed, this,
                [](void)
        {
           interfaceButtons |= 1;
        });

        connect(homeButton, &QPushButton::released, this,
                [](void)
        {
           interfaceButtons &= ~1;
        });

        connect(powerButton, &QPushButton::pressed, this,
                [](void)
        {
           interfaceButtons |= 2;
        });

        connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this,
            [](int deviceId, QGamepadManager::GamepadButton button)
        {
            (void)deviceId;

            gpConfigurator->setCurDeviceId(deviceId);

            if(gpConfigurator->isVisible())
            {
               gpConfigurator->getInput(deviceId, button);
                return;
            }

                buttons &= QGamepadManager::GamepadButtons(~(1 << button));
        });

        connect(gpConfigurator->skipButton, &QPushButton::released, this,
                [](void)
        {
               gpConfigurator->next();
        });


        connect(gpConfigurator->resetConfigButton, &QPushButton::released, this,
                [](void)
        {
               QMessageBox *msgBox = new QMessageBox(0);
               QGamepadManager::instance()->resetConfiguration(gpConfigurator->getCurDeviceId());

               msgBox->setText("Reset");
               msgBox->setInformativeText("Please restart the program for changes to take affect.");
               msgBox->show();

        });

        connect(powerButton, &QPushButton::released, this,
                [](void)
        {
           interfaceButtons &= ~2;
        });

        connect(longPowerButton, &QPushButton::pressed, this,
                [](void)
        {
           interfaceButtons |= 4;
        });

        connect(longPowerButton, &QPushButton::released, this,
                [](void)
        {
           interfaceButtons &= ~4;
        });

        connect(settingsConfigButton, &QPushButton::released, this,
                [this](void)
        {
            if (!settingsConfig->isVisible())
            {
                settingsConfig->move(this->x() - settingsConfig->width() - 5,this->y());
                settingsConfig->show();
            } else settingsConfig->hide();
        });

        /*connect(clearImageButton, &QPushButton::released, this,
                [this](void)
        {
           touchScreen->clearImage();
        });*/

        /*connect(touchOpacitySlider, &QSlider::valueChanged, this,
                [this](int value)
        {
            touchScreen->setWindowOpacity(value / 10.0);
            touchScreen->update();
        });*/

        //touchScreen = new TouchScreen(nullptr);
        settingsConfig = new ConfigWindow(nullptr, touchScreen);
        this->setWindowTitle(tr("InputRedirectionClient-Qt"));

        addrLineEdit->setText(settings.value("ipAddress", "").toString());
    }

    void show(void)
    {
        QWidget::show();
        //touchScreen->move(this->x() + this->width() + 5,this->y());
        //touchScreen->show();
        settingsConfig->hide();
    }

    //When closing, save shortcuts
    void closeEvent(QCloseEvent *ev)
    {
        //Save shortcuts
        unsigned int i = 0;
        for (i=0; i<listShortcuts.size(); i++)
         {
            QString valName = tr("tsShortcut%1").arg(i);
             settings.setValue(valName, qVariantFromValue(listShortcuts[i]));
         }

        //Remove leftover settings
        for(; settings.contains(tr("tsShortcut%1").arg(i)); i++)
         {
             settings.remove(tr("tsShortcut%1").arg(i));
         }
        //touchScreen->close();
        settingsConfig->close();

        //touchScreen->setTouchScreenPressed(false);
        //delete touchScreen;
        ev->accept();
    }

    //Move touchscreen window with main window if moved
    void moveEvent(QMoveEvent *event)
    {
        //touchScreen->move(touchScreen->pos() + (event->pos() - event->oldPos()));
    }

    //When main window is opened, load shortcut settings
    void showEvent(QShowEvent *event)
    {
        qRegisterMetaType<ShortCut>("ShortCut");
        qRegisterMetaTypeStreamOperators<ShortCut>("ShortCut");

    QString valName = "tsShortcut0";
    for(int i = 0; settings.contains(valName); i++)
     {
        valName = tr("tsShortcut%1").arg(i);
        QVariant variant = settings.value(valName);

        ShortCut curShort = variant.value<ShortCut>();
         if(variant.isValid() && curShort.name != "")
         {
             if(curShort.name != "")
             {
              listShortcuts.push_back(curShort);
             }

         }
         else
         {
             settings.remove(tr("tsShortcut%1").arg(i));
         }

     }
    }

    virtual ~Widget(void)
    {
        worker.setLeftAxis(0.0, 0.0);
        worker.setRightAxis(0.0, 0.0);

        buttons = 0;
        interfaceButtons = 0;

        delete settingsConfig;
    }
};

#endif // MAINWIDGET_H
