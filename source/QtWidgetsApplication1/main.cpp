//#include "QtWidgetsApplication1.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include <vector>
#include <qlabel.h>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDirIterator>
#include <QListWidget>

#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QFileIconProvider>
#include <QDesktopServices>

#include <QCoreApplication>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QLineEdit>
#include <QTimer>
#include <QGridLayout>
#include <QMenu>
#include <QtWidgets>
#include <QInputMethod>


#include <QGuiApplication>

#include <QByteArray>


#include <QMainWindow>

#include <QEvent>
#include <QCompleter>
#include <QFileDialog>


#include "keyboard.h"
#include "Controller.h"
#include "CursorMode.h"
#include "GetKeyboardInput.h"








void AllocateConsole(const LPCSTR console_title)
{
    AllocConsole();
    SetConsoleTitleA(console_title);

    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCout, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
}




// Some global controller variables
bool controller_navigation = true;
// Used to do some stuff once during the first loop in ControllerNavigation()
bool temp = false;
bool init_gamepad = false;
// Global palette for the UI
QPalette ui_palette;









// https://stackoverflow.com/questions/12459145/extracting-icon-using-winapi-in-qt-app

QIcon GetFileIcon(const QString& file_path)
{
    SHFILEINFO shfi;
    memset(&shfi, 0, sizeof(SHFILEINFO));

    //if (SHGetFileInfo(reinterpret_cast<const wchar_t*>(file_path.utf16()), 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES))
    //if (SHGetFileInfo(reinterpret_cast<const wchar_t*>(file_path.utf16()), 0, &shfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES))
    if (SHGetFileInfo(reinterpret_cast<const wchar_t*>(file_path.utf16()), 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
    {
        // Scale the icon to a desired size
        QPixmap pixmap = QPixmap::fromImage(QImage::fromHICON(shfi.hIcon))\
            .scaled(QSize(PercentToWidth(6.66), PercentToHeight(11.85)), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QIcon icon(pixmap);

        // Cleanup the icon resource
        DestroyIcon(shfi.hIcon);

        return icon;
    }

    
    return QIcon();
}








QString RemoveFileExtension(const QString& file_name) 
{
    int last_dot = file_name.lastIndexOf('.');

    if (last_dot != -1)
    {
        // Return without file extension
        return file_name.left(last_dot);
    }

    // Return the original
    return file_name;
}











class ApplicationExplorer : public QWidget
{


    
public:
    VirtualKeyboard* QKeyboard;
    ApplicationExplorer(QApplication& app, QWidget* parent = nullptr) : QWidget(parent), app(app)
    {
        
        
        // Controller task loop - Perform task constantly
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ApplicationExplorer::TaskGamepadNavigation);
        timer->start();

    }


    void CreateUI() 
    { 

        LoadAppData();
        setLayout(layout_root);
        if (is_first_launch)
        {
            SetupIntroScreen();
        }
        else
        {
            //LoadProfile();
            SetupUI();
            OnApplicationLaunch();
        }
        
        
    }




    void OnApplicationLaunch()
    {
        showMaximized();
        QEventLoop loop;
        QTimer::singleShot(5, &loop, &QEventLoop::quit);

        // Execute the event loop, blocking until the timer fires
        loop.exec();

        // Fixes vertical icons bug
        SearchApplication(line_all_search, list_widget, applications_list);
        
    }

private:
    bool is_first_launch = true;
    QStringList applications_list;
    QStringList favorites_list;
    QListWidget* list_widget = new QListWidget(this);
    QListWidget* list_favorites = new QListWidget(this);
    QListWidget* search_list = new QListWidget(this);
    QListWidget* directory_list = new QListWidget(this);
    QVBoxLayout* layout_root = new QVBoxLayout(this);
    QVBoxLayout* search_layout;
    QLineEdit* search_bar;
    QTimer* timer;


    QLineEdit* line_all_search;
    QLineEdit* line_fav_search;

    // Reference to QApplication object
    QApplication& app; 
    QComboBox* cb_theme;
    QComboBox* cb_profile_switch = new QComboBox(this);
    QCheckBox* chb_file_extension;
    QCheckBox* chb_icon_mode;
    QCheckBox* chb_wrapping;

    QTimer timer_search;
    QTimer* communication_task_timer;

    QGridLayout* layout_all_apps;

    QTabWidget* tabs;
    QPushButton* button_add_tab;
    QPushButton* button_remove_tab;
    QWidget search_window;
    


    // QMap to store usage frequency of each application
    QMap<QString, int> app_usage_freq; 
    QStringList popular_apps;
    QCompleter* completer;



    void UpdatePopularAppsList()
    {
        // Sort application file paths based on usage frequency
        QList<QString> sorted_apps = app_usage_freq.keys();
        std::sort(sorted_apps.begin(), sorted_apps.end(), [&](const QString& a, const QString& b) 
            {
                return app_usage_freq[a] > app_usage_freq[b];
            });

        // Get top 10 most used applications
        popular_apps = sorted_apps.mid(0, 10);
    }

    
    
    void SetupUI()
    {
        
        
        QFont font("Arial", 18);
        

        tabs = new QTabWidget(this);
        // Tabs
        QWidget* tab_all_apps = new QWidget();
        QWidget* tab_favorites = new QWidget();
        //QWidget* tab_popular = new QWidget();
        QWidget* tab_settings = new QWidget();
       
        //tab_all_apps->setToolTip("A list with all of your applications");
        //tab_favorites->setToolTip("A list with your favorite applications");
        //tab_settings->setToolTip("Personalize the launcher");
        

        QComboBox* cb_sort_by = new QComboBox(this);
        cb_sort_by->addItem("Ascending");
        cb_sort_by->addItem("Descending");
        cb_sort_by->addItem("Popularity");
        
        

        // Layouts
        //QVBoxLayout* layout_all_apps = new QVBoxLayout(tab_all_apps);
        layout_all_apps = new QGridLayout(tab_all_apps);
        QGridLayout* layout_favorites = new QGridLayout(tab_favorites);
        //QGridLayout* layout_popular = new QGridLayout(tab_popular);
        QVBoxLayout* layout_settings = new QVBoxLayout(tab_settings);
        //QVBoxLayout* layout_intro = new QVBoxLayout(tab_intro);


        
        





        // Add tabs to tab widget
        tabs->addTab(tab_all_apps, "Applications");
        tabs->addTab(tab_favorites, "Favorites");
        //tabs->addTab(tab_favorites, "Popular");
        tabs->addTab(tab_settings, "Settings");
        //tabs->addTab(tab_intro, "Welcome!");
        
        layout_root->addWidget(tabs);

        


        // List widget custom arguments
        list_favorites->setViewMode(QListView::IconMode);
        list_favorites->setMovement(QListView::Static);
        list_favorites->setWordWrap(true);
        list_favorites->setWrapping(true);
        list_favorites->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        list_favorites->setSpacing(40);
        list_favorites->setIconSize(QSize(64, 64));
        list_favorites->sortItems();


        
        
        

        // Create the second tab
        QLabel* label_directories = new QLabel("Directories:");
        QLabel* label_app_theme = new QLabel("Theme");
        QLabel* label_profile = new QLabel("Profile");

        QPushButton* button_add_dir = new QPushButton("Add Directory or Application");
        QPushButton* button_remove_dir = new QPushButton("Remove Selected Item");
        QPushButton* button_delete_profile = new QPushButton("Delete Current Profile");
        QPushButton* button_virtual_keyb = new QPushButton("Open Virtual Keyboard");
        button_delete_profile->setStyleSheet("text-align: left; color: red;");
        button_remove_dir->setStyleSheet("color: red;");
        button_add_dir->setToolTip("Add a directory in which the launcher will be looking for applications to execute.\
The launcher is looking for executables and local or internet shortcut files.");
        button_remove_dir->setToolTip("Remove the currently selected directory from the application launcher.");
        
        



        cb_theme = new QComboBox(this);
        cb_theme->setToolTip("Change the launcher's theme.");
        cb_theme->addItem("Dark");
        cb_theme->addItem("Light");
        cb_theme->addItem("Red");
        cb_theme->addItem("Green");
        cb_theme->addItem("Blue");
        cb_theme->addItem("Yellow");
        cb_theme->addItem("Orange");
        cb_theme->addItem("Purple");


        // File extension checkbox
        chb_file_extension = new QCheckBox("Include File Extension", this);
        chb_file_extension->setToolTip("Include file extension in application text.");
        chb_icon_mode = new QCheckBox("Icon Mode", this);
        chb_icon_mode->setToolTip("Display items as icons instead of lists.");
        chb_wrapping = new QCheckBox("Wrapping", this);
        chb_wrapping->setToolTip("Wrap list items.");
        
        
        

        QLineEdit* line_add_profile = new QLineEdit(this);
        line_add_profile->setPlaceholderText("Create New Profile");
        line_add_profile->setFixedHeight(PercentToHeight(2.32));
        
        
        tab_settings->setFont(font);
        //button_add_dir->setFont(font);
        //button_remove_dir->setFont(font);
        //directory_list->setFont(font);
        //cb_theme->setFont(font);
        //cb_profile_switch->setFont(font);
        button_delete_profile->setFont(font);
        button_remove_dir->setFont(font);
        //chb_file_extension->setFont(font);
        //line_add_profile->setFont(font);

        QHBoxLayout* layout_dir_buttons = new QHBoxLayout();
        layout_dir_buttons->addWidget(button_add_dir);
        layout_dir_buttons->addWidget(button_remove_dir);


        QVBoxLayout* layout_about = new QVBoxLayout();
        // Create a QLabel for the header
        QLabel* header_keyb_launcher = new QLabel("Keyboard application launcher");
        header_keyb_launcher->setStyleSheet("font-size: 16pt; font-weight: bold;");

        // Create a QLabel for the regular text
        QLabel* label_keyb_launcher = new QLabel("\
A keyboard application launcher is hidden within the functionality of this application. Keyboard users can use the\n\
 application launcher which is designed specifically for keyboard use only. In order to access the window with the\n\
 functionality mentioned you need to press CTRL + Space. Then you'll see a window with an input field. Then you can\n\
 proceed with typing the application that you're looking for and finally use the arrows to navigate through the\n\
 applications that have been found below. The application will be executed by hitting Return (Enter) key on the\n\
 keyboard.The keyboard application launcher is looking for applications in the same directories given by the user\n\
 in Settings.");
        QLabel* header_controller_nav = new QLabel("Xbox Gamepad navigation");
        header_keyb_launcher->setStyleSheet("font-size: 16pt; font-weight: bold;");

        QLabel* label_controller_nav = new QLabel("\
The final device supported from this application is the Xbox Gamepad. Much like mouses, the Xbox controllers are\n\
 taking advantage of the main application window in which all application are displayed as icons. Below you can learn\n\
 how to use the application using a controller handled by XInput.");
        


        //layout_about->addWidget(header_keyb_launcher);
        //layout_about->addWidget(label_keyb_launcher);
        //layout_about->addWidget(header_controller_nav);
        //layout_about->addWidget(label_controller_nav);


        layout_settings->addWidget(button_virtual_keyb);
        layout_settings->addWidget(chb_file_extension);
        layout_settings->addWidget(chb_icon_mode);
        layout_settings->addWidget(chb_wrapping);
        layout_settings->addWidget(label_profile);
        layout_settings->addWidget(cb_profile_switch);
        layout_settings->addWidget(button_delete_profile);
        layout_settings->addWidget(line_add_profile);
        layout_settings->addWidget(label_app_theme);
        layout_settings->addWidget(cb_theme);
        layout_settings->addWidget(label_directories);
        layout_settings->addWidget(directory_list);
        layout_settings->addLayout(layout_dir_buttons);
        layout_settings->addLayout(layout_about);

        



        

        // Input field for application searching
        line_all_search = new QLineEdit(this);
        line_all_search->setPlaceholderText("Search application");
        line_all_search->setFixedHeight(PercentToHeight(2.32));
        line_all_search->setFont(font);

        line_fav_search = new QLineEdit(this);
        line_fav_search->setPlaceholderText("Search application");
        line_fav_search->setFixedHeight(PercentToHeight(2.32));
        line_fav_search->setFont(font);


        

        // List widget custom arguments
        list_widget->setViewMode(QListView::IconMode);
        list_widget->setMovement(QListView::Static);
        list_widget->setWordWrap(true);
        list_widget->setWrapping(true);
        list_widget->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        list_widget->setSpacing(40);
        list_widget->setIconSize(QSize(64, 64));
        //list_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //list_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        list_widget->sortItems();

        
        layout_all_apps->addWidget(line_all_search);
        layout_all_apps->addWidget(cb_sort_by);
        layout_all_apps->addWidget(list_widget);
        layout_favorites->addWidget(line_fav_search);
        layout_favorites->addWidget(list_favorites);
        
        

        // Load last used profile
        LoadProfile();
        connect(cb_theme, &QComboBox::currentTextChanged, this, &ApplicationExplorer::UpdateUIPalette);
        connect(cb_profile_switch, &QComboBox::currentTextChanged, this, &ApplicationExplorer::LoadProfile);
        connect(cb_sort_by, &QComboBox::currentTextChanged, this, &ApplicationExplorer::SortingChanged);


        //for (const QString& app : popular_apps) { std::cout << app.toStdString() << std::endl; }
        UpdatePopularAppsList();
        completer = new QCompleter(popular_apps);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        line_all_search->setCompleter(completer);

        



        // CONNECTIONS
        QObject::connect(&app, &QCoreApplication::aboutToQuit, this, &ApplicationExplorer::OnApplicationExit);
        connect(list_widget, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        connect(list_favorites, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        
        //QObject::connect(cb_profile_switch, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ApplicationExplorer::SaveProfile);

        connect(chb_file_extension, &QCheckBox::stateChanged, this, [this] {
            ApplicationExplorer::UpdateListWidget(applications_list, list_widget); 
            ApplicationExplorer::UpdateListWidget(favorites_list, list_favorites); 
            });
        connect(chb_icon_mode, &QCheckBox::stateChanged, this, [this] {
            if (chb_icon_mode->isChecked())
            {
                list_widget->setViewMode(QListWidget::IconMode);
                list_favorites->setViewMode(QListWidget::IconMode);
            }
            else
            {
                list_widget->setViewMode(QListWidget::ListMode);
                list_favorites->setViewMode(QListWidget::ListMode);
            }
            });


        connect(chb_wrapping, &QCheckBox::stateChanged, this, [this] {
            list_widget->setWrapping(chb_wrapping->isChecked());
            list_favorites->setWrapping(chb_wrapping->isChecked());
            });
        
        connect(button_virtual_keyb, &QPushButton::clicked, this, [this] {QKeyboard->showMaximized(); });


        //// Delay before keyboard input triggers application search
        timer_search.setSingleShot(true);
        //connect(&timer_search, &QTimer::timeout, this, [this] {ApplicationExplorer::UpdateListWidget(applications_list, list_widget); });
        connect(line_all_search, &QLineEdit::textChanged, this, [this] {ApplicationExplorer::SearchApplication(line_all_search, list_widget, applications_list); });
        connect(line_fav_search, &QLineEdit::textChanged, this, [this] {ApplicationExplorer::SearchApplication(line_fav_search, list_favorites, favorites_list); });
        
        connect(button_delete_profile, &QPushButton::clicked, this, [this] {
                if (cb_profile_switch->count() > 1)
                {
                    QString output = "Are you sure that you want to delete profile \"" + cb_profile_switch->currentText() + "\"?\n\
All data in this profile will be permanently deleted.";
                    int ret = QMessageBox::question(this, "Confirmation Dialog", output, QMessageBox::Yes | QMessageBox::No);

                    if (ret == QMessageBox::Yes)
                    {
                        // Delete profile
                        cb_profile_switch->removeItem(cb_profile_switch->currentIndex());
                        // Clear profile data on disk
                        QSettings settings("ThesisOrganization", "QtWidgetsApp1");
                        settings.beginGroup("group_name");
                        settings.clear();
                        settings.endGroup();

                    }
                    else
                    {
                        // Skip deleting profile
                        return;
                    }
                }
                else
                {
                    QMessageBox::information(this, "Information", "Cannot delete all user profiles...");
                }
            });
        connect(line_add_profile, &QLineEdit::returnPressed, this, [this, line_add_profile] {
            if (line_add_profile->text() != "")
            {
                int index = cb_profile_switch->findText(line_add_profile->text());

                if (index != -1) 
                {
                    QMessageBox::information(this, "Username exists", "This username already exists. Try again!");
                    return;
                }
                else 
                {
                    cb_profile_switch->addItem(line_add_profile->text());
                    cb_profile_switch->setCurrentIndex(cb_profile_switch->count() - 1);
                    QString output = "Profile \"" + line_add_profile->text() + "\" has been successfully created!";
                    line_add_profile->clear();
                    QMessageBox::information(this, "Profile creation completed", output);
                }
            }
            });


        // Custom context menu for the list widgets
        list_widget->setContextMenuPolicy(Qt::CustomContextMenu);
        list_favorites->setContextMenuPolicy(Qt::CustomContextMenu);
        
        
        connect(list_widget, &QListWidget::customContextMenuRequested, this, &ApplicationExplorer::ShowContextMenuForListWidget);
        connect(list_favorites, &QListWidget::customContextMenuRequested, this, &ApplicationExplorer::ShowContextMenuForListFavorites);




        // Functionality to add directories
        QObject::connect(button_add_dir, &QPushButton::clicked, [this]() {

            QString directory = QFileDialog::getExistingDirectory(nullptr, "Select Directory", QDir::homePath());
            for (int i = 0; i < directory_list->count(); ++i)
            {

                if (directory == directory_list->item(i)->text())
                {
                    return;
                }
            }
            if (!directory.isEmpty())
                directory_list->addItem(directory);
            DirectoryListUpdated();
            UpdateListWidget(applications_list, list_widget);
            SaveProfile();
            });

        // Functionality to remove directories
        QObject::connect(button_remove_dir, &QPushButton::clicked, [this]() {
            qDeleteAll(directory_list->selectedItems());
            DirectoryListUpdated();
            UpdateListWidget(applications_list, list_widget);
            ApplicationExplorer::SaveProfile();
            });

        
        
        // ULauncher-like window for application searching through the keyboard
        search_window.setWindowTitle("Qt6ULauncher");
        search_layout = new QVBoxLayout();
        //QVBoxLayout* sea_layout = new QVBoxLayout();
        search_window.setLayout(search_layout);
        search_window.setStyleSheet("background-color: transparent;");
        search_window.resize(400, 100);//(PercentToWidth(7.81), PercentToHeight(1.85));
        //search_window.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        search_window.setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        //search_window.setWindowFlags(Qt::Popup);
        

        // Create UI elements
        search_bar = new QLineEdit(this);
        search_bar->setFixedHeight(70);//(PercentToHeight(2.32));
        search_bar->setFont(QFont("Arial", 26));
        //search_bar->setFocusPolicy()

        search_list->setViewMode(QListView::ListMode);
        search_list->hide();
        search_list->setFixedHeight(120);//(PercentToHeight(2.32));
        search_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        search_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        search_list->setFont(font);
        
        search_layout->addWidget(search_bar);
        search_layout->addWidget(search_list);
        
        connect(search_bar, &QLineEdit::textChanged, this, &ApplicationExplorer::SearchResults);
        connect(search_list, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        

        communication_task_timer = new QTimer(this);
        connect(communication_task_timer, &QTimer::timeout, this, &ApplicationExplorer::TaskHandleDevicesUICommunication);
        // Perform task constantly
        communication_task_timer->start(0);

        DirectoryListUpdated();
        UpdateListWidget(applications_list, list_widget);

        
    }

    
    
    void TaskHandleDevicesUICommunication()
    {


        // Handling keyboard application launcher window hardware - software communication
        if (IsKeyDown(VK_CONTROL) && IsKeyJustDown(VK_SPACE))
        {
            if (search_window.isVisible())
            {
                search_window.hide();
            }
            else
            {
                

                search_window.show();
                //search_window.raise();
                //search_window.activateWindow();
                //search_window.setFocus();
                
                //search_bar->setEnabled(true);
                search_bar->setFocus();
                search_bar->clear();
            }

            return;
        }



        if (!search_list->hasFocus() && (IsKeyJustDown(VK_DOWN) || IsKeyJustDown(VK_UP)))
        {
            //search_list->setFocusPolicy(Qt::NoFocus);
            search_list->setCurrentRow(0);
            
            search_list->setFocus();
        }


        // Checks to return to keyboard input
        for (int i = 'A'; i <= 'Z'; ++i)
        {
            if (IsKeyJustDown(i))
            {
                search_bar->setFocus();
            }
        }

        if (IsKeyJustDown(VK_SPACE) || IsKeyJustDown(VK_BACK) || IsKeyJustDown(VK_LEFT) || IsKeyJustDown(VK_RIGHT))
        {
            search_bar->setFocus();
        }



        // Handling controller navigation - UI communication


        // Focus on the search input field
        if (user->IsButtonJustDown(GAMEPAD_DPAD_UP) && list_widget->currentRow() <= 12)
        {
            line_all_search->setFocus();
        }

        // Focus on the applications list
        if (user->IsButtonJustDown(GAMEPAD_DPAD_DOWN) && line_all_search->hasFocus())
        {
            list_widget->setFocus();
        }


        // LB - go to the left tab
        if (user->IsButtonJustDown(GAMEPAD_LB))
        {
            if (tabs->currentIndex() > 0)
            {
                tabs->setCurrentIndex(tabs->currentIndex() - 1);
            }
        }


        // LB - go to the right tab
        if (user->IsButtonJustDown(GAMEPAD_RB))
        {
            if (tabs->currentIndex() < 2)
            {
                tabs->setCurrentIndex(tabs->currentIndex() + 1);
            }
        }




    }


    
    // Searching for the ULauncher-like window
    void SearchResults(const QString& text)
    {
        search_list->clear();

        if (text.isEmpty())
        {
            search_list->hide();
            search_window.resize(400, 100);
            search_layout->update();
            search_list->update();
            return;
        }

        int max_results = 10;
        int count = 0;

        
        for (int i = 0; i < list_widget->count(); ++i)
        {
            QListWidgetItem* item = list_widget->item(i);

            if (item->text().contains(text, Qt::CaseInsensitive))
            {

                QString path = item->data(Qt::UserRole).toString();
                QFileInfo file_info(path);
                QIcon icon = GetFileIcon(path);

                QListWidgetItem* item2;

                // Create list widget item using the file name or file extension
                if (chb_file_extension->isChecked())
                {
                    item2 = new QListWidgetItem(icon, file_info.fileName());
                }
                else
                {
                    item2 = new QListWidgetItem(icon, RemoveFileExtension(file_info.fileName()));
                }


                // Store file path as item data
                item2->setData(Qt::UserRole, path);
                search_list->addItem(item2);
                
                ++count;
                if (count >= max_results)
                {
                    break;
                }
            }
        }


        if (search_list->count() == 0)
        {
            search_list->hide();
        }
        else
        {
            search_list->show();
        }

        search_layout->update();
        search_list->update();
    }


    
    void SaveProfile()
    {
        QSettings settings("ThesisOrganization", "QtWidgetsApp1");

        settings.beginGroup(cb_profile_switch->itemText(cb_profile_switch->currentIndex()));

        settings.setValue("Theme", cb_theme->currentIndex());
        settings.setValue("icon_mode", chb_icon_mode->isChecked());
        settings.setValue("wrapping", chb_wrapping->isChecked());
        
        // Convert the QMap to a QVariantMap
        QVariantMap map;
        QMapIterator<QString, int> iter(app_usage_freq);
        while (iter.hasNext()) 
        {
            iter.next();
            map.insert(iter.key(), iter.value());
        }
        settings.setValue("app_frequency", map);

        QStringList directories;

        // Iterate through the directory_list and save directory paths to QStringList
        for (int i = 0; i < directory_list->count(); ++i)
        {
            directories << directory_list->item(i)->text();
        }

        // Save directories to settings
        settings.setValue("Directories", directories);


        settings.endGroup();
    }


    void LoadProfile()
    {

        QSettings settings("ThesisOrganization", "QtWidgetsApp1");
        
        settings.beginGroup(cb_profile_switch->itemText(cb_profile_switch->currentIndex()));

        
        //popular_apps = settings.value("popular_apps").toStringList();
        QVariantMap retrievedMap = settings.value("app_frequency").toMap();

        // Convert the QVariantMap back to a QMap<QString, int>
        QMapIterator<QString, QVariant> mapIter(retrievedMap);
        while (mapIter.hasNext()) 
        {
            mapIter.next();
            app_usage_freq.insert(mapIter.key(), mapIter.value().toInt());
        }

        int index = settings.value("Theme", 0).toInt();
        cb_theme->setCurrentIndex(index);
        UpdateUIPalette(cb_theme->itemText(index));


        QStringList directories = settings.value("Directories").toStringList();

        // Clear the listWidget before loading directories
        directory_list->clear();

        // Add directories from settings to the listWidget
        foreach(const QString & dir, directories)
        {
            directory_list->addItem(dir);
        }


        chb_icon_mode->setChecked(settings.value("icon_mode", true).toBool());
        chb_wrapping->setChecked(settings.value("wrapping", true).toBool());
        list_widget->setViewMode(chb_icon_mode->isChecked() ? QListWidget::IconMode : QListWidget::ListMode);
        list_favorites->setViewMode(chb_icon_mode->isChecked() ? QListWidget::IconMode : QListWidget::ListMode);
        list_widget->setWrapping(chb_wrapping->isChecked());
        list_favorites->setWrapping(chb_wrapping->isChecked());

        //DirectoryListUpdated();
        //UpdateListWidget(applications_list, list_widget);
        

        settings.endGroup();
    }



    void SaveAppData()
    {
        QSettings settings("ThesisOrganization", "QtWidgetsApp1");
        
        settings.beginGroup("application_default");
        
        settings.setValue("last_profile", cb_profile_switch->currentIndex());
        settings.setValue("is_first_launch", is_first_launch);

        QStringList profile_items;

        for (int i = 0; i < cb_profile_switch->count(); ++i)
        {
            profile_items.append(cb_profile_switch->itemText(i));
        }
        settings.setValue("profiles", profile_items);
        
        settings.endGroup();
    }


    void LoadAppData()
    {
        QSettings settings("ThesisOrganization", "QtWidgetsApp1");
        
        settings.beginGroup("application_default");


        is_first_launch = settings.value("is_first_launch", true).toBool();
        

        // Might be a problem on first launch
        // Trying to read items on first launch might cause an issue
        if (!is_first_launch)
        {
            QStringList profile_items = settings.value("profiles").toStringList();
            cb_profile_switch->clear();
            cb_profile_switch->addItems(profile_items);


            // Load the saved index of the combo box
            int index = settings.value("last_profile", 0).toInt();
            cb_profile_switch->setCurrentIndex(index);
        }
        

        // End reading settings
        settings.endGroup();
        
    }



    void OnApplicationExit()
    {
        ApplicationExplorer::SaveProfile();
        Beep(200, 200);
    }



    void ShowContextMenuForListWidget(const QPoint& pos){ ShowContextMenu(pos, list_widget); }
    void ShowContextMenuForListFavorites(const QPoint& pos){ ShowContextMenu(pos, list_favorites); }



    void ShowContextMenu(const QPoint& pos, QListWidget* list)
    {
        /*
        QPoint local_pos = list->mapFromGlobal(pos);

        // Get the index of the item at the local position
        QModelIndex index = list->indexAt(local_pos);

        // Check if the right-click occurred on an item
        if (!index.isValid())
        {
            return;
        } */

        QListWidgetItem* item = list->currentItem();
        if (!item)
            return;
        
        QMenu context_menu(tr("Context Menu"), this);

        QAction* action_edit = new QAction(tr("Add Application"), this);
        connect(action_edit, &QAction::triggered, this, [this, item]() {
            
            QFileDialog dialog;
            dialog.setWindowTitle("Select application file");
            dialog.setNameFilters({ "Shortcut files (*.lnk)", "Executable files (*.exe)", "URL files (*.url)" });
            dialog.setFileMode(QFileDialog::ExistingFile);

            // Execute the file dialog and check if the user selected a file
            if (dialog.exec() == QDialog::Accepted)
            {
                QString file_selected = dialog.selectedFiles().first();
                
                std::cout << "File selected: " << file_selected.toStdString() << std::endl;

                for (int i = 0; i < directory_list->count(); ++i)
                {

                    if (file_selected == directory_list->item(i)->text())
                    {
                        return;
                    }
                }
                if (!file_selected.isEmpty())
                    directory_list->addItem(file_selected);

                DirectoryListUpdated();
                UpdateListWidget(applications_list, list_widget);
                SaveProfile();

            }
            });


        



        bool found = false;

        for (int i = 0; i < list_favorites->count(); ++i)
        {

            QListWidgetItem* currentItem = list_favorites->item(i);

            if (currentItem->text() == item->text())
            {

                found = true;
                break;
            }
        }

        QAction* action_add_favorite;
        if (found)
        {
            action_add_favorite = new QAction(tr("Remove from favorites"), this);

            connect(action_add_favorite, &QAction::triggered, this, [this, item]()
                {
                    if (!item)
                    {
                        std::cout << "Invalid item" << std::endl;
                        return;
                    }

                    QString path = item->data(Qt::UserRole).toString();
                    //QFileInfo file_info(path);

                    int index_fav = favorites_list.indexOf(path);
                    if (index_fav != -1)
                    {
                        favorites_list.removeAt(index_fav);
                        UpdateListWidget(favorites_list, list_favorites);
                    }



                    return;


                    int string_index = -1;
                    std::cout << "Target: " << item->data(Qt::UserRole).toString().toStdString() << std::endl;
                    for (int i = 0; i < favorites_list.count(); ++i)
                    {
                        std::cout << "Index: " << i << " Item: " << favorites_list[i].toStdString() << std::endl;
                        if (item->data(Qt::UserRole) == favorites_list[i])
                        {
                            std::cout << "Found at index: " << i << std::endl;
                            string_index = i;
                            break;
                        }
                    }


                    if (string_index != -1)
                    {
                        std::cout << "Removing at: " << string_index << std::endl;
                        favorites_list.removeAt(string_index);

                        UpdateListWidget(favorites_list, list_favorites);
                    }
                    else
                    {
                        std::cout << "Favorites list string index was not found" << std::endl;
                    }
                    return;
                    int index = list_favorites->row(item);
                    QListWidgetItem* item_to_remove = list_favorites->takeItem(index);
                    delete item_to_remove;

                });

        }
        else
        {
            action_add_favorite = new QAction(tr("Add to favorites"), this);

            connect(action_add_favorite, &QAction::triggered, this, [this, item]()
                {
                    if (!item)
                    {
                        std::cout << "Invalid item" << std::endl;
                        return;
                    }

                    
                    QFileInfo file_info(item->data(Qt::UserRole).toString());
                    if (!favorites_list.contains(file_info.filePath()))
                    {
                        favorites_list.append(file_info.filePath());
                        UpdateListWidget(favorites_list, list_favorites);
                    }

                });
        }




        context_menu.addAction(action_edit);
        context_menu.addAction(action_add_favorite);


        //context_menu.exec(mapToGlobal(pos));
        context_menu.exec(QCursor::pos());

    }


    



    void ExploreDirectoryFiles(const QString& path)
    {
        QDir dir(path);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QFileInfo& file_info : files)
        {
            if (file_info.isDir())
            {
                // Recursive call for subdirectories
                ExploreDirectoryFiles(file_info.filePath());
            }
            else
            {
                QString filename = file_info.fileName();
                if (filename.endsWith(".exe") || filename.endsWith(".lnk") || filename.endsWith(".url"))
                {
                    if (!applications_list.contains(file_info.filePath()))
                    {
                        applications_list.append(file_info.filePath());
                    }
                    
                }
            }
        }
    }





    void UpdateListWidget(QStringList current_list, QListWidget* list_wid)
    {
        list_wid->clear();
        // Add items to the list widget
        for (const QString& path : current_list)
        {
            QFileInfo file_info(path);
            QIcon icon = GetFileIcon(path);

            QListWidgetItem* item;

            // Create list widget item using the file name or file extension
            if (chb_file_extension->isChecked())
            {
                item = new QListWidgetItem(icon, file_info.fileName());
            }
            else
            {
                item = new QListWidgetItem(icon, RemoveFileExtension(file_info.fileName()));
            }


            // Store file path as item data
            item->setData(Qt::UserRole, path);
            list_wid->addItem(item);

        }

        list_wid->update();

        
    }

    

    // Start or restart the timer when text changes - helps avoiding performance issues
    void SearchApplication(QLineEdit* line, QListWidget* list_wid, QStringList app_list)
    {
        
        if (line->text() == "")
        {
            UpdateListWidget(app_list, list_wid);
            timer_search.start(200);
            return;
        }

        // Update the QStringList based on the input on the search bar
        
        QStringList new_list;
        for (const QString& item : app_list)
        {
            if (item.toLower().contains(line->text().toLower()))
            {
                new_list << item;
            }
        }

        UpdateListWidget(new_list, list_wid);
        timer_search.start(200);

    }



    void ExecuteApplication(QListWidgetItem* item) 
    {



        if (item) 
        {
            // Hide the ulauncher-like window if the application was executed through that
            if (search_window.isVisible())
            {
                search_window.hide();
            }


            QString file_path = item->data(Qt::UserRole).toString();
            QString file_name = RemoveFileExtension(item->text());
            
            
            
            if (file_path.endsWith(".lnk", Qt::CaseInsensitive) || file_path.endsWith(".url", Qt::CaseInsensitive))
            {
                // Execute shortcuts using QDesktopServices
                if (!QDesktopServices::openUrl(QUrl::fromLocalFile(file_path)))
                {
                    std::cout << "Failed to open shortcut file:" << file_path.toStdString();
                    return;
                }
                
            }
            else if(file_path.endsWith(".exe", Qt::CaseInsensitive))
            {
                // If it's not a shortcut create a QProcess and start it
                QProcess* process = new QProcess();
                process->start(file_path);

                // Connect signals for asynchronous process handling
                QObject::connect(process, &QProcess::started, []() { });

                QObject::connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                    [](int exitCode, QProcess::ExitStatus exitStatus) {
                        std::cout << "Process finished with exit code:" << exitCode << "and exit status:" << exitStatus;
                    });
                QObject::connect(process, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
                    std::cout << "Error occurred:" << error;
                    });
            }
            else
            {
                return;
            }

            app_usage_freq[file_name]++;
        }
    }



    void DirectoryListUpdated()
    {
        applications_list.clear();

        // Update directories list
        for (int i = 0; i < directory_list->count(); ++i)
        {

            QListWidgetItem* item = directory_list->item(i);
            QString path = item->text();
            
            if (item)
            {
                QFileInfo file_info(path);
                if (file_info.exists())
                {
                    if (file_info.isDir()) 
                    {
                        ExploreDirectoryFiles(path);
                    }
                    else if (file_info.isFile()) 
                    {
                        if (path.endsWith(".exe") || path.endsWith(".lnk") || path.endsWith(".url"))
                        {
                            if (!applications_list.contains(file_info.filePath()))
                            {
                                applications_list.append(file_info.filePath());
                            }

                        }
                    }
                    else 
                    {
                        QString output = "This isn't a file: " + path;
                        QMessageBox::information(this, "Information", output);
                    }
                }
                else 
                {
                    QString output = "This item doesn't exist: " + path;
                    QMessageBox::information(this, "Information", output);
                }
                
            }
        }
    }





    void SetupIntroScreen()
    {
        /*this->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                this->size(),
                app.primaryScreen()->availableGeometry()
            )
        );*/

        QFont font("Arial", 26);
        QFont font2("Arial", 18);


        QLabel* title = new QLabel("First time setup", this);
        title->setAlignment(Qt::AlignCenter);
        title->setFont(font);


        QLabel* label_username = new QLabel("Welcome to the Application Launcher!\n\
This is the first time setup window.\n\
You can type in your own user name or\n\
use the default given below.", this);
        label_username->setAlignment(Qt::AlignCenter);
        label_username->setFont(font2);
        

        QLineEdit* line_username = new QLineEdit(this);
        line_username->setFixedHeight(PercentToHeight(2.32));
        line_username->setFixedWidth(PercentToWidth(12.32));
        
        line_username->setFont(font2);
        //line_username->setAlignment(Qt::AlignCenter);
        line_username->setText("Profile_1");

        
        

        QPushButton* continueButton = new QPushButton("Continue", this);
        continueButton->setFixedSize(300, 100);
        continueButton->setFont(font);
        connect(continueButton, &QPushButton::clicked, this, [title, label_username, line_username, continueButton, this] 
            {
            std::cout << "Username: " << line_username->text().toStdString() << std::endl;
            cb_profile_switch->addItem(line_username->text());
            cb_profile_switch->setCurrentIndex(0);
            is_first_launch = false;
            SaveAppData();
            layout_root->removeWidget(title);
            layout_root->removeWidget(label_username);
            layout_root->removeWidget(line_username);
            layout_root->removeWidget(continueButton);

            delete title;
            delete label_username;
            delete line_username;
            delete continueButton;
            CreateUI();
            });

        

        layout_root->addWidget(title);
        layout_root->addWidget(label_username);
        layout_root->addWidget(line_username);
        layout_root->addWidget(continueButton, 0, Qt::AlignCenter);
        
    }

    

    


    
    void SortingChanged(const QString& text)
    {
        if (text == "Ascending")
        {
            list_widget->sortItems(Qt::AscendingOrder);
        }
        else if (text == "Descending")
        {
            list_widget->sortItems(Qt::DescendingOrder);

        }
        else if (text == "Popularity")
        {
            QList<QString> sorted_apps = app_usage_freq.keys();
            std::sort(sorted_apps.begin(), sorted_apps.end(), [&](const QString& a, const QString& b)
                {
                    return app_usage_freq[a] > app_usage_freq[b];
                });

            for (int k = 0; k < sorted_apps.count(); ++k)
            {
                std::cout << sorted_apps[k].toStdString() << std::endl;
            }
            return;
            
            for (int i = 0; i < sorted_apps.count(); ++i)
            {
                for (int j = 0; j < list_widget->count(); ++j)
                {
                    QListWidgetItem* item1 = list_widget->takeItem(j);
                    QString path = item1->data(Qt::UserRole).toString();
                    QFileInfo file_info(path);
                    QString file_name = file_info.fileName();
                    file_name = RemoveFileExtension(file_name);

                    if (sorted_apps[i] == file_name)
                    {
                        //new_list->addItem(item);

                        list_widget->insertItem(i, item1);

                        //QListWidgetItem* item2 = list_widget->takeItem(i + 1);
                        //list_widget->insertItem(j, item2);
                        
                    }
                }

            }

            //list_widget = new_list;
        }
    }







    void UpdateUIPalette(const QString &text)
    {


        if(text == "Dark")
        {
            ui_palette.setColor(QPalette::Window, QColor(30, 30, 30)); // Main window background area
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(45, 45, 45)); // List layout box area
            ui_palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53)); 
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::white); // Text color in combo boxes etc
            ui_palette.setColor(QPalette::Button, QColor(53, 53, 53)); // Background color of buttons, combo boxes etc
            ui_palette.setColor(QPalette::ButtonText, Qt::white); // Button text
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::HighlightedText, Qt::white); // Highlighted text in input fields

            
        }
        else if(text == "Light")
        {
            ui_palette.setColor(QPalette::Window, Qt::white);
            ui_palette.setColor(QPalette::WindowText, Qt::black);
            ui_palette.setColor(QPalette::Base, QColor(110, 110, 110));
            ui_palette.setColor(QPalette::AlternateBase, Qt::lightGray);
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::black);
            ui_palette.setColor(QPalette::Button, QColor(110, 110, 110));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::HighlightedText, Qt::white);
            
        }
        else if(text == "Blue")
        {
            ui_palette.setColor(QPalette::Window, QColor(20, 20, 40));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(30, 30, 60));
            ui_palette.setColor(QPalette::AlternateBase, QColor(20, 20, 40));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(30, 30, 60));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(0, 160, 240));
            ui_palette.setColor(QPalette::Highlight, QColor(0, 160, 240));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if(text == "Green")
        {
            ui_palette.setColor(QPalette::Window, QColor(20, 40, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(30, 60, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(20, 40, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(30, 60, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(0, 160, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(0, 160, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if(text == "Red")
        {
            ui_palette.setColor(QPalette::Window, QColor(40, 20, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(60, 30, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(40, 20, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(60, 30, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(160, 0, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(160, 0, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if (text == "Yellow")
        {
            ui_palette.setColor(QPalette::Window, QColor(40, 40, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(60, 60, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(40, 40, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(60, 60, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(160, 160, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(160, 160, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if(text == "Orange")
        {
            ui_palette.setColor(QPalette::Window, QColor(60, 40, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(90, 60, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(60, 40, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(90, 60, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(255, 128, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(255, 128, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);

        }
        else if(text == "Purple")
        {
            ui_palette.setColor(QPalette::Window, QColor(40, 20, 40));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(60, 30, 60));
            ui_palette.setColor(QPalette::AlternateBase, QColor(40, 20, 40));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(60, 30, 60));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(128, 0, 128));
            ui_palette.setColor(QPalette::Highlight, QColor(128, 0, 128));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }


        app.setPalette(ui_palette);
    }


    









    void ControllerNavigation()
    {

        //Toggle controller navigation (the user wouldn't want the controller navigation to be working while gaming)
        if (user->IsButtonDown(GAMEPAD_BACK) && user->IsButtonJustDown(GAMEPAD_RIGHT_THUMB))
        {
            controller_navigation = !controller_navigation;
            Beep(523, 500);
        }



        //Open the virtual keboard to type an input and send it
        if (user->IsButtonDown(GAMEPAD_BACK) && user->IsButtonJustDown(GAMEPAD_RB))
        {
            getting_input = true;
            //QKeyboard->show();
            QKeyboard->showMaximized();
        }


        if (getting_input)
        {
            QKeyboard->SendKeyboardInput();
        }




        if (controller_navigation)
        {
            // Used in Smooth scrolling/navigating with the arrows/D-Pad --- must be called once every loop
            ResetKeyPressCounter();
            CursorMode();
        }

    }


    void TaskGamepadNavigation()
    {

        // Perform gamepad initialization (once)
        if (!init_gamepad)
        {
            user = new XBOXController(1); // Initizialize the controller object
            init_gamepad = true;
        }


        if (user->IsConnected())
        {
            // Getting trigger values
            user->left_trigger = user->GetState().Gamepad.bLeftTrigger / 255.0f;
            user->right_trigger = user->GetState().Gamepad.bRightTrigger / 255.0f;
            ControllerNavigation();

        }
    }
    
};





class Worker : public QObject 
{

public slots:
    void DoWork() 
    {
        while (true) 
        {
            // Your loop code here
            qDebug() << "Worker iteration";
            QThread::sleep(1); // Sleep to simulate some work and prevent busy-waiting
            // Break the loop if a condition is met
            // if (conditionToStop) break;
        }

        emit finished();
    }

signals:
    void finished();
};








int main(int argc, char* argv[]) 
{
    AllocateConsole("Debug console for Qt6");
    

    QApplication app(argc, argv);
    
    ApplicationExplorer explorer(app);
    explorer.setWindowTitle("P2019140 - Konstantinos Tourtsakis");
    explorer.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);
    explorer.resize(800, 600);
    
    // Storing screen resolution
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);
    std::cout << "Screen resolution: " << screen_width << "x" << screen_height << std::endl;

    explorer.CreateUI();
    
    VirtualKeyboard QKeyboard;
    //QKeyboard.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);
    explorer.QKeyboard = &QKeyboard;
    
    
    
    
    return app.exec();
}





