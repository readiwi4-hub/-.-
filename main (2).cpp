#include <QApplication>
#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QLineEdit>
#include <QToolBar>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QAction>
#include <QStyle>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QWebEngineSettings>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QFont>
#include <QSplitter>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QPlainTextEdit>

class CodeEditor : public QWidget {
    Q_OBJECT

private:
    QPlainTextEdit* editor;
    QTextEdit* output;
    QComboBox* languageSelector;
    QPushButton* runButton;
    QPushButton* clearButton;
    QPushButton* saveButton;
    QPushButton* loadButton;

public:
    CodeEditor(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(5, 5, 5, 5);

        // Верхняя панель инструментов
        QHBoxLayout* toolbar = new QHBoxLayout();
        
        QLabel* langLabel = new QLabel("Язык:", this);
        toolbar->addWidget(langLabel);

        languageSelector = new QComboBox(this);
        languageSelector->addItem("Python");
        languageSelector->addItem("JavaScript");
        languageSelector->addItem("C++");
        languageSelector->addItem("HTML");
        toolbar->addWidget(languageSelector);

        toolbar->addStretch();

        loadButton = new QPushButton("Открыть", this);
        toolbar->addWidget(loadButton);

        saveButton = new QPushButton("Сохранить", this);
        toolbar->addWidget(saveButton);

        runButton = new QPushButton("▶ Запустить", this);
        runButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 5px 15px; }");
        toolbar->addWidget(runButton);

        clearButton = new QPushButton("Очистить", this);
        toolbar->addWidget(clearButton);

        mainLayout->addLayout(toolbar);

        // Splitter для редактора и вывода
        QSplitter* splitter = new QSplitter(Qt::Vertical, this);

        // Редактор кода
        editor = new QPlainTextEdit(this);
        QFont font("Courier New", 11);
        editor->setFont(font);
        editor->setPlaceholderText("Введите код здесь...");
        editor->setStyleSheet("QPlainTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #3e3e3e; }");
        
        // Пример кода по умолчанию
        editor->setPlainText("# Python example\nprint('Hello from Code Editor!')\n\nfor i in range(5):\n    print(f'Number: {i}')");

        splitter->addWidget(editor);

        // Область вывода
        QWidget* outputWidget = new QWidget(this);
        QVBoxLayout* outputLayout = new QVBoxLayout(outputWidget);
        outputLayout->setContentsMargins(0, 0, 0, 0);

        QLabel* outputLabel = new QLabel("Вывод:", this);
        outputLayout->addWidget(outputLabel);

        output = new QTextEdit(this);
        output->setReadOnly(true);
        output->setFont(font);
        output->setStyleSheet("QTextEdit { background-color: #0c0c0c; color: #cccccc; border: 1px solid #3e3e3e; }");
        outputLayout->addWidget(output);

        splitter->addWidget(outputWidget);
        splitter->setStretchFactor(0, 3);
        splitter->setStretchFactor(1, 1);

        mainLayout->addWidget(splitter);

        // Подключение сигналов
        connect(runButton, &QPushButton::clicked, this, &CodeEditor::runCode);
        connect(clearButton, &QPushButton::clicked, this, &CodeEditor::clearOutput);
        connect(saveButton, &QPushButton::clicked, this, &CodeEditor::saveCode);
        connect(loadButton, &QPushButton::clicked, this, &CodeEditor::loadCode);
        connect(languageSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &CodeEditor::onLanguageChanged);
    }

    void runCode() {
        QString code = editor->toPlainText();
        QString language = languageSelector->currentText();
        
        output->clear();
        output->append("=== Выполнение кода ===\n");

        if (language == "Python") {
            runPython(code);
        } else if (language == "JavaScript") {
            runJavaScript(code);
        } else if (language == "C++") {
            output->append("Компиляция и запуск C++ требует установленного компилятора.\n");
            runCpp(code);
        } else if (language == "HTML") {
            output->append("HTML код будет отображен в браузере.\n");
            // Можно открыть HTML в новой вкладке браузера
        }
    }

    void runPython(const QString& code) {
        // Создаем временный файл
        QString tempPath = QDir::temp().filePath("temp_code.py");
        QFile file(tempPath);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << code;
            file.close();

            // Запускаем Python
            QProcess* process = new QProcess(this);
            connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
                output->append(QString::fromUtf8(process->readAllStandardOutput()));
            });
            connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
                output->append("<span style='color: red;'>" + 
                             QString::fromUtf8(process->readAllStandardError()) + 
                             "</span>");
            });
            connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, [this, process](int exitCode, QProcess::ExitStatus status) {
                output->append(QString("\n=== Завершено с кодом: %1 ===").arg(exitCode));
                process->deleteLater();
            });

            process->start("python3", QStringList() << tempPath);
        } else {
            output->append("Ошибка: не удалось создать временный файл.");
        }
    }

    void runJavaScript(const QString& code) {
        QString tempPath = QDir::temp().filePath("temp_code.js");
        QFile file(tempPath);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << code;
            file.close();

            QProcess* process = new QProcess(this);
            connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
                output->append(QString::fromUtf8(process->readAllStandardOutput()));
            });
            connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
                output->append("<span style='color: red;'>" + 
                             QString::fromUtf8(process->readAllStandardError()) + 
                             "</span>");
            });
            connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, [this, process](int exitCode, QProcess::ExitStatus status) {
                output->append(QString("\n=== Завершено с кодом: %1 ===").arg(exitCode));
                process->deleteLater();
            });

            process->start("node", QStringList() << tempPath);
        } else {
            output->append("Ошибка: не удалось создать временный файл.");
        }
    }

    void runCpp(const QString& code) {
        QString tempPath = QDir::temp().filePath("temp_code.cpp");
        QString exePath = QDir::temp().filePath("temp_code");
        
        QFile file(tempPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << code;
            file.close();

            // Компиляция
            QProcess* compileProcess = new QProcess(this);
            connect(compileProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, [this, compileProcess, exePath](int exitCode, QProcess::ExitStatus status) {
                if (exitCode == 0) {
                    output->append("Компиляция успешна. Запуск...\n");
                    
                    // Запуск
                    QProcess* runProcess = new QProcess(this);
                    connect(runProcess, &QProcess::readyReadStandardOutput, this, [this, runProcess]() {
                        output->append(QString::fromUtf8(runProcess->readAllStandardOutput()));
                    });
                    connect(runProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                            this, [this, runProcess](int exitCode, QProcess::ExitStatus status) {
                        output->append(QString("\n=== Завершено с кодом: %1 ===").arg(exitCode));
                        runProcess->deleteLater();
                    });
                    runProcess->start(exePath);
                } else {
                    output->append("<span style='color: red;'>Ошибка компиляции:\n" + 
                                 QString::fromUtf8(compileProcess->readAllStandardError()) + 
                                 "</span>");
                }
                compileProcess->deleteLater();
            });

            compileProcess->start("g++", QStringList() << tempPath << "-o" << exePath);
        }
    }

    void clearOutput() {
        output->clear();
    }

    void saveCode() {
        QString fileName = QFileDialog::getSaveFileName(this, 
            "Сохранить код", 
            "", 
            "All Files (*);;Python Files (*.py);;JavaScript Files (*.js);;C++ Files (*.cpp);;HTML Files (*.html)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << editor->toPlainText();
                file.close();
                output->append(QString("Файл сохранен: %1\n").arg(fileName));
            }
        }
    }

    void loadCode() {
        QString fileName = QFileDialog::getOpenFileName(this, 
            "Открыть код", 
            "", 
            "All Files (*);;Python Files (*.py);;JavaScript Files (*.js);;C++ Files (*.cpp);;HTML Files (*.html)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                editor->setPlainText(in.readAll());
                file.close();
                output->append(QString("Файл загружен: %1\n").arg(fileName));
            }
        }
    }

    void onLanguageChanged(int index) {
        QString language = languageSelector->currentText();
        
        if (language == "Python") {
            editor->setPlainText("# Python example\nprint('Hello, Python!')\n");
        } else if (language == "JavaScript") {
            editor->setPlainText("// JavaScript example\nconsole.log('Hello, JavaScript!');\n");
        } else if (language == "C++") {
            editor->setPlainText("#include <iostream>\n\nint main() {\n    std::cout << \"Hello, C++!\" << std::endl;\n    return 0;\n}\n");
        } else if (language == "HTML") {
            editor->setPlainText("<!DOCTYPE html>\n<html>\n<head>\n    <title>My Page</title>\n</head>\n<body>\n    <h1>Hello, HTML!</h1>\n</body>\n</html>\n");
        }
    }
};

class Browser : public QMainWindow {
    Q_OBJECT

private:
    QTabWidget* tabWidget;
    QLineEdit* addressBar;
    QPushButton* backButton;
    QPushButton* forwardButton;
    QPushButton* reloadButton;
    QPushButton* newTabButton;
    QPushButton* codeEditorButton;

public:
    Browser(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("My Browser");
        resize(1200, 800);

        // Центральный виджет
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout* layout = new QVBoxLayout(centralWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // Панель инструментов
        QToolBar* toolbar = new QToolBar(this);
        toolbar->setMovable(false);
        layout->addWidget(toolbar);

        // Кнопки навигации
        backButton = new QPushButton(style()->standardIcon(QStyle::SP_ArrowBack), "", this);
        forwardButton = new QPushButton(style()->standardIcon(QStyle::SP_ArrowForward), "", this);
        reloadButton = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload), "", this);
        newTabButton = new QPushButton("+", this);
        codeEditorButton = new QPushButton("</> Код", this);
        codeEditorButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 5px 10px; }");

        toolbar->addWidget(backButton);
        toolbar->addWidget(forwardButton);
        toolbar->addWidget(reloadButton);

        // Адресная строка
        addressBar = new QLineEdit(this);
        addressBar->setPlaceholderText("Введите URL...");
        toolbar->addWidget(addressBar);

        // Кнопка редактора кода
        toolbar->addWidget(codeEditorButton);
        
        // Кнопка новой вкладки
        toolbar->addWidget(newTabButton);

        // Виджет вкладок
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        layout->addWidget(tabWidget);

        // Подключение сигналов
        connect(backButton, &QPushButton::clicked, this, &Browser::goBack);
        connect(forwardButton, &QPushButton::clicked, this, &Browser::goForward);
        connect(reloadButton, &QPushButton::clicked, this, &Browser::reload);
        connect(newTabButton, &QPushButton::clicked, this, &Browser::addNewTab);
        connect(codeEditorButton, &QPushButton::clicked, this, &Browser::openCodeEditor);
        connect(addressBar, &QLineEdit::returnPressed, this, &Browser::navigateToUrl);
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        connect(tabWidget, &QTabWidget::currentChanged, this, &Browser::onTabChanged);

        // Создаем первую вкладку
        addNewTab();
    }

    void addNewTab() {
        QWebEngineView* webView = new QWebEngineView(this);
        
        // Настройки для веб-движка
        webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
        webView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        webView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
        
        // Подключение к прокси через localhost:5000
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName("localhost");
        proxy.setPort(5000);
        QNetworkProxy::setApplicationProxy(proxy);

        int index = tabWidget->addTab(webView, "Новая вкладка");
        tabWidget->setCurrentIndex(index);

        // Обновление заголовка вкладки при загрузке страницы
        connect(webView, &QWebEngineView::titleChanged, this, [this, webView](const QString& title) {
            int idx = tabWidget->indexOf(webView);
            if (idx != -1) {
                tabWidget->setTabText(idx, title.isEmpty() ? "Новая вкладка" : title);
            }
        });

        // Обновление адресной строки при изменении URL
        connect(webView, &QWebEngineView::urlChanged, this, [this, webView](const QUrl& url) {
            if (tabWidget->currentWidget() == webView) {
                addressBar->setText(url.toString());
            }
        });

        // Загружаем стартовую страницу
        webView->setUrl(QUrl("http://google.com"));
        addressBar->setFocus();
    }

    void closeTab(int index) {
        if (tabWidget->count() > 1) {
            QWidget* tab = tabWidget->widget(index);
            tabWidget->removeTab(index);
            delete tab;
        } else {
            // Если закрываем последнюю вкладку, закрываем браузер
            close();
        }
    }

    void onTabChanged(int index) {
        if (index >= 0) {
            QWebEngineView* currentView = qobject_cast<QWebEngineView*>(tabWidget->widget(index));
            if (currentView) {
                addressBar->setText(currentView->url().toString());
                
                // Обновляем состояние кнопок навигации
                backButton->setEnabled(currentView->page()->action(QWebEnginePage::Back)->isEnabled());
                forwardButton->setEnabled(currentView->page()->action(QWebEnginePage::Forward)->isEnabled());
            }
        }
    }

    void navigateToUrl() {
        QString url = addressBar->text();
        
        // Если не указан протокол, добавляем http://
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://" + url;
        }

        QWebEngineView* currentView = qobject_cast<QWebEngineView*>(tabWidget->currentWidget());
        if (currentView) {
            currentView->setUrl(QUrl(url));
        }
    }

    void goBack() {
        QWebEngineView* currentView = qobject_cast<QWebEngineView*>(tabWidget->currentWidget());
        if (currentView) {
            currentView->back();
        }
    }

    void goForward() {
        QWebEngineView* currentView = qobject_cast<QWebEngineView*>(tabWidget->currentWidget());
        if (currentView) {
            currentView->forward();
        }
    }

    void reload() {
        QWebEngineView* currentView = qobject_cast<QWebEngineView*>(tabWidget->currentWidget());
        if (currentView) {
            currentView->reload();
        }
    }

    void openCodeEditor() {
        CodeEditor* editor = new CodeEditor(this);
        int index = tabWidget->addTab(editor, "📝 Редактор кода");
        tabWidget->setCurrentIndex(index);
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    Browser browser;
    browser.show();

    return app.exec();
}

#include "main.moc"
