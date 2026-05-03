#include "coinremovalgui.h"

#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QFont>
#include <QSizePolicy>
#include <QDateTime>
#include <QApplication>
#include <chrono>

// ═══════════════════════════════════════════════════════════════════════════════
//  CoinWidget
// ═══════════════════════════════════════════════════════════════════════════════

CoinWidget::CoinWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void CoinWidget::setState(char state)
{
    if (m_state == state) return;
    m_state = state;
    startFlash();
    update();
}

void CoinWidget::setFlashOpacity(qreal v)
{
    m_flashOpacity = v;
    update();
}

void CoinWidget::startFlash()
{
    auto *anim = new QPropertyAnimation(this, "flashOpacity", this);
    anim->setDuration(400);
    anim->setKeyValueAt(0.0, 0.8);
    anim->setKeyValueAt(0.5, 0.3);
    anim->setKeyValueAt(1.0, 0.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void CoinWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF rect = QRectF(4, 4, width() - 8, height() - 8);

    // Shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 40));
    p.drawEllipse(rect.adjusted(2, 3, 2, 3));

    // Body colour
    QColor bodyColor;
    QColor borderColor;
    QString label;
    QColor  labelColor;
    if (m_state == 'H') {
        bodyColor   = QColor(56, 161, 105);   // green
        borderColor = QColor(38, 120, 78);
        label       = "H";
        labelColor  = Qt::white;
    } else if (m_state == 'T') {
        bodyColor   = QColor(229, 62, 62);    // red
        borderColor = QColor(185, 28, 28);
        label       = "T";
        labelColor  = Qt::white;
    } else {
        bodyColor   = QColor(160, 160, 160);  // grey (removed)
        borderColor = QColor(110, 110, 110);
        label       = "✕";
        labelColor  = QColor(80, 80, 80);
    }

    // Gradient fill
    QRadialGradient grad(rect.center() - QPointF(rect.width() * 0.15,
                                                  rect.height() * 0.2),
                         rect.width() * 0.7);
    grad.setColorAt(0.0, bodyColor.lighter(140));
    grad.setColorAt(1.0, bodyColor);
    p.setBrush(grad);
    p.setPen(QPen(borderColor, 2));
    p.drawEllipse(rect);

    // Flash overlay
    if (m_flashOpacity > 0.0) {
        p.setBrush(QColor(255, 255, 255, static_cast<int>(m_flashOpacity * 200)));
        p.setPen(Qt::NoPen);
        p.drawEllipse(rect);
    }

    // Label – use pixel size for consistent rendering across different DPI settings
    QFont font = p.font();
    font.setPixelSize(static_cast<int>(rect.height() * 0.30));
    font.setBold(true);
    p.setFont(font);
    p.setPen(labelColor);
    p.drawText(rect, Qt::AlignCenter, label);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CoinRemovalWindow  –  static algorithm helpers
// ═══════════════════════════════════════════════════════════════════════════════

/*static*/ std::string CoinRemovalWindow::stateToString(const std::vector<char>& c)
{
    return std::string(c.begin(), c.end());
}

/*static*/ bool CoinRemovalWindow::isSolved(const std::vector<char>& c)
{
    for (char x : c) if (x != '_') return false;
    return true;
}

/*static*/ bool CoinRemovalWindow::hasHeads(const std::vector<char>& c)
{
    for (char x : c) if (x == 'H') return true;
    return false;
}

/*static*/ void CoinRemovalWindow::flip(char& coin)
{
    coin = (coin == 'H') ? 'T' : 'H';
}

/*static*/ bool CoinRemovalWindow::solve(std::vector<char>& coins,
                                          std::set<std::string>& visited,
                                          std::vector<int>& moves)
{
    if (isSolved(coins)) return true;

    const std::string state = stateToString(coins);
    if (visited.count(state)) return false;
    if (!hasHeads(coins)) { visited.insert(state); return false; }

    for (int i = 0; i < static_cast<int>(coins.size()); ++i) {
        if (coins[i] == 'H') {
            std::vector<char> backup = coins;
            coins[i] = '_';
            if (i > 0 && coins[i-1] != '_') flip(coins[i-1]);
            if (i < static_cast<int>(coins.size())-1 && coins[i+1] != '_') flip(coins[i+1]);
            moves.push_back(i);
            if (solve(coins, visited, moves)) return true;
            moves.pop_back();
            coins = backup;
        }
    }
    visited.insert(state);
    return false;
}

bool CoinRemovalWindow::solveCoinRemoval(std::vector<char> coins,
                                          std::vector<int>& solution)
{
    int heads = 0;
    for (char c : coins) if (c == 'H') ++heads;
    if (heads % 2 == 0) return false;          // even heads → never solvable

    std::set<std::string> visited;
    std::vector<int>      moves;
    if (solve(coins, visited, moves)) {
        solution = moves;
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CoinRemovalWindow  –  test-case data
// ═══════════════════════════════════════════════════════════════════════════════

/*static*/ std::vector<CoinRemovalWindow::TestCase> CoinRemovalWindow::buildTestCases()
{
    return {
        {"Test  1 – 7 coins, 5 heads",                   {'T','H','H','T','H','H','H'}},
        {"Test  2 – 1 coin, 1 head",                     {'H'}},
        {"Test  3 – 3 coins, 3 heads",                   {'H','H','H'}},
        {"Test  4 – 5 coins, 3 heads",                   {'H','T','H','T','H'}},
        {"Test  5 – 5 coins, 5 heads",                   {'H','H','H','H','H'}},
        {"Test  6 – 3 coins, 2 heads (even)",            {'H','T','H'}},
        {"Test  7 – 4 coins, 4 heads (even)",            {'H','H','H','H'}},
        {"Test  8 – 10 coins, 5 heads",                  {'T','H','T','H','T','H','T','H','T','H'}},
        {"Test  9 – 6 coins, 2 heads (even)",            {'H','T','T','T','T','H'}},
        {"Test 10 – 9 coins, 9 heads",                   {'H','H','H','H','H','H','H','H','H'}},
        {"Test 11 – 5 coins, 4 heads (even)",            {'H','H','T','H','H'}},
        {"Test 12 – 6 coins, 4 heads (even)",            {'H','T','H','H','T','H'}},
        {"Test 13 – 9 coins, 3 heads",                   {'T','T','H','T','T','H','T','T','H'}},
        {"Test 14 – 3 coins, 2 heads (even) [same as Test 6]", {'H','T','H'}},
        {"Test 15 – 3 coins, 1 head",                    {'T','H','T'}},
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CoinRemovalWindow  –  UI construction
// ═══════════════════════════════════════════════════════════════════════════════

CoinRemovalWindow::CoinRemovalWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_testCases = buildTestCases();
    buildUi();
    loadTestCase(0);
}

void CoinRemovalWindow::buildUi()
{
    setWindowTitle("Coin Removal Algorithm – Backtracking Visualiser");
    setMinimumSize(750, 640);

    // ── Central widget ────────────────────────────────────────────────────────
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setSpacing(12);
    rootLayout->setContentsMargins(16, 16, 16, 16);

    // ── Stylesheet ────────────────────────────────────────────────────────────
    qApp->setStyle("Fusion");
    setStyleSheet(R"(
        QMainWindow  { background: #1a1a2e; }
        QWidget      { background: #1a1a2e; color: #e0e0e0; font-family: 'Segoe UI', sans-serif; }
        QGroupBox    { border: 1px solid #3a3a5c; border-radius: 8px;
                       margin-top: 8px; padding: 8px;
                       font-weight: bold; color: #a0a8d0; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
        QPushButton  { background: #2d2d4e; border: 1px solid #5a5a8c;
                       border-radius: 6px; padding: 6px 18px;
                       color: #d0d8ff; font-weight: bold; }
        QPushButton:hover   { background: #3d3d6e; border-color: #8080c0; }
        QPushButton:pressed { background: #1d1d3e; }
        QPushButton:disabled{ background: #1a1a2e; color: #505060; border-color: #303050; }
        QPushButton#solveBtn { background: #2a5f3f; border-color: #4a9f6f; color: #90ffbf; }
        QPushButton#solveBtn:hover { background: #3a7f5f; }
        QPushButton#stepBtn  { background: #2a3f6f; border-color: #4a6faf; color: #90b0ff; }
        QPushButton#stepBtn:hover  { background: #3a5f9f; }
        QPushButton#resetBtn { background: #5a3a1a; border-color: #9f6a3a; color: #ffcf90; }
        QPushButton#resetBtn:hover { background: #7a5a3a; }
        QPushButton#clearBtn { background: #3a1a1a; border-color: #8f3a3a; color: #ff9090; }
        QPushButton#clearBtn:hover { background: #5a2a2a; }
        QComboBox    { background: #2d2d4e; border: 1px solid #5a5a8c;
                       border-radius: 5px; padding: 4px 8px; color: #d0d8ff; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background: #2d2d4e; color: #d0d8ff;
                                       selection-background-color: #4040a0; }
        QTextEdit    { background: #12122a; border: 1px solid #3a3a5c;
                       border-radius: 6px; color: #c0c8e0; font-family: monospace; }
        QLabel       { color: #c0c8e0; }
        QScrollArea  { border: none; background: transparent; }
        QStatusBar   { background: #12122a; color: #8090b0; font-size: 12px; }
    )");

    // ── Test-case selection ───────────────────────────────────────────────────
    QGroupBox *caseGroup = new QGroupBox("Test Case", central);
    QVBoxLayout *caseLayout = new QVBoxLayout(caseGroup);

    m_caseCombo = new QComboBox(caseGroup);
    for (const auto &tc : m_testCases)
        m_caseCombo->addItem(QString::fromStdString(tc.description));
    caseLayout->addWidget(m_caseCombo);

    QHBoxLayout *caseInfoRow = new QHBoxLayout;
    m_headsLabel   = new QLabel(caseGroup);
    m_solvableLabel= new QLabel(caseGroup);
    m_caseDescLabel= new QLabel(caseGroup);
    m_caseDescLabel->setWordWrap(true);
    caseInfoRow->addWidget(m_headsLabel);
    caseInfoRow->addWidget(m_solvableLabel);
    caseInfoRow->addStretch();
    caseLayout->addLayout(caseInfoRow);
    caseLayout->addWidget(m_caseDescLabel);
    rootLayout->addWidget(caseGroup);

    // ── Coin display ──────────────────────────────────────────────────────────
    QGroupBox *coinGroup = new QGroupBox("Coin State", central);
    QVBoxLayout *coinGroupLayout = new QVBoxLayout(coinGroup);

    QScrollArea *scrollArea = new QScrollArea(coinGroup);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFixedHeight(100);

    m_coinContainer = new QWidget;
    m_coinContainer->setStyleSheet("background: transparent;");
    m_coinLayout = new QHBoxLayout(m_coinContainer);
    m_coinLayout->setSpacing(8);
    m_coinLayout->setContentsMargins(8, 8, 8, 8);
    m_coinLayout->addStretch();

    scrollArea->setWidget(m_coinContainer);
    coinGroupLayout->addWidget(scrollArea);
    rootLayout->addWidget(coinGroup);

    // ── Controls ──────────────────────────────────────────────────────────────
    QGroupBox *ctrlGroup = new QGroupBox("Controls", central);
    QHBoxLayout *ctrlLayout = new QHBoxLayout(ctrlGroup);
    ctrlLayout->setSpacing(10);

    m_solveBtn = new QPushButton("▶  Solve", ctrlGroup);
    m_solveBtn->setObjectName("solveBtn");
    m_stepBtn  = new QPushButton("⏭  Step",  ctrlGroup);
    m_stepBtn->setObjectName("stepBtn");
    m_resetBtn = new QPushButton("↺  Reset", ctrlGroup);
    m_resetBtn->setObjectName("resetBtn");
    m_clearBtn = new QPushButton("✕  Clear", ctrlGroup);
    m_clearBtn->setObjectName("clearBtn");

    for (auto *btn : {m_solveBtn, m_stepBtn, m_resetBtn, m_clearBtn}) {
        btn->setFixedHeight(36);
        btn->setMinimumWidth(100);
        ctrlLayout->addWidget(btn);
    }
    ctrlLayout->addStretch();
    rootLayout->addWidget(ctrlGroup);

    // ── Current-move highlight ────────────────────────────────────────────────
    m_currentMoveLabel = new QLabel("No move applied yet.", central);
    m_currentMoveLabel->setAlignment(Qt::AlignCenter);
    m_currentMoveLabel->setStyleSheet(
        "font-size:15px; font-weight:bold; color:#f6d860; "
        "padding:6px; border-radius:5px; background:#25253a;");
    rootLayout->addWidget(m_currentMoveLabel);

    // ── Log / results ─────────────────────────────────────────────────────────
    QGroupBox *logGroup = new QGroupBox("Results", central);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    m_logEdit = new QTextEdit(logGroup);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMinimumHeight(160);
    m_logEdit->setFont(QFont("Courier New", 10));
    logLayout->addWidget(m_logEdit);
    rootLayout->addWidget(logGroup);

    // ── Status bar ────────────────────────────────────────────────────────────
    m_statusBar = statusBar();
    m_statusBar->showMessage("Ready");

    // ── Signal connections ────────────────────────────────────────────────────
    connect(m_caseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CoinRemovalWindow::onTestCaseChanged);
    connect(m_solveBtn, &QPushButton::clicked, this, &CoinRemovalWindow::onSolve);
    connect(m_stepBtn,  &QPushButton::clicked, this, &CoinRemovalWindow::onStep);
    connect(m_resetBtn, &QPushButton::clicked, this, &CoinRemovalWindow::onReset);
    connect(m_clearBtn, &QPushButton::clicked, this, &CoinRemovalWindow::onClear);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CoinRemovalWindow  –  slots
// ═══════════════════════════════════════════════════════════════════════════════

void CoinRemovalWindow::onTestCaseChanged(int index)
{
    loadTestCase(index);
}

void CoinRemovalWindow::onSolve()
{
    m_logEdit->clear();
    m_currentMoveLabel->setText("Solving…");
    QApplication::processEvents();

    const auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<int> solution;
    const bool ok = solveCoinRemoval(m_initialCoins, solution);
    const auto t1 = std::chrono::high_resolution_clock::now();
    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!ok) {
        appendLog("No solution exists for this configuration.");
        setStatusMsg("No solution found.", true);
        m_currentMoveLabel->setText("No solution exists.");
        m_currentMoveLabel->setStyleSheet(
            "font-size:15px; font-weight:bold; color:#ff6060; "
            "padding:6px; border-radius:5px; background:#25253a;");
        return;
    }

    // Apply every move visually and log them
    m_currentCoins = m_initialCoins;
    appendLog(QString("Solution found in %1 ms  (%2 move%3):")
              .arg(ms, 0, 'f', 3)
              .arg(solution.size())
              .arg(solution.size() == 1 ? "" : "s"));

    for (int step = 0; step < static_cast<int>(solution.size()); ++step) {
        const int idx = solution[step];
        applyMove(idx);
        appendLog(QString("  Move %1: remove coin at position %2  →  [%3]")
                  .arg(step + 1)
                  .arg(idx)
                  .arg(QString::fromStdString(stateToString(m_currentCoins))));
        QApplication::processEvents();
    }

    m_solution      = solution;
    m_stepIndex     = static_cast<int>(solution.size());
    m_solutionReady = true;

    appendLog("\nAll coins removed – puzzle solved!");
    setStatusMsg(QString("Solved in %1 ms").arg(ms, 0, 'f', 3));
    m_currentMoveLabel->setText("✔  Puzzle solved – all coins removed!");
    m_currentMoveLabel->setStyleSheet(
        "font-size:15px; font-weight:bold; color:#60ff90; "
        "padding:6px; border-radius:5px; background:#25253a;");
    m_stepBtn->setEnabled(false);
}

void CoinRemovalWindow::onStep()
{
    // Pre-solve on first step
    if (!m_solutionReady) {
        std::vector<int> solution;
        const bool ok = solveCoinRemoval(m_initialCoins, solution);
        if (!ok) {
            appendLog("No solution exists – nothing to step through.");
            setStatusMsg("No solution found.", true);
            m_currentMoveLabel->setText("No solution exists.");
            m_currentMoveLabel->setStyleSheet(
                "font-size:15px; font-weight:bold; color:#ff6060; "
                "padding:6px; border-radius:5px; background:#25253a;");
            m_stepBtn->setEnabled(false);
            return;
        }
        m_solution      = solution;
        m_stepIndex     = 0;
        m_solutionReady = true;
        appendLog(QString("Solution has %1 move%2. Press Step to apply each move.")
                  .arg(solution.size())
                  .arg(solution.size() == 1 ? "" : "s"));
    }

    if (m_stepIndex >= static_cast<int>(m_solution.size())) {
        setStatusMsg("All steps already applied.");
        return;
    }

    const int idx = m_solution[m_stepIndex];
    applyMove(idx);
    ++m_stepIndex;

    m_currentMoveLabel->setText(
        QString("Step %1/%2 – removed coin at position %3")
        .arg(m_stepIndex).arg(m_solution.size()).arg(idx));
    m_currentMoveLabel->setStyleSheet(
        "font-size:15px; font-weight:bold; color:#f6d860; "
        "padding:6px; border-radius:5px; background:#25253a;");

    appendLog(QString("Step %1: remove coin at position %2  →  [%3]")
              .arg(m_stepIndex)
              .arg(idx)
              .arg(QString::fromStdString(stateToString(m_currentCoins))));

    if (m_stepIndex == static_cast<int>(m_solution.size())) {
        appendLog("\nAll coins removed – puzzle solved!");
        setStatusMsg("Puzzle solved!");
        m_currentMoveLabel->setText("✔  Puzzle solved – all coins removed!");
        m_currentMoveLabel->setStyleSheet(
            "font-size:15px; font-weight:bold; color:#60ff90; "
            "padding:6px; border-radius:5px; background:#25253a;");
        m_stepBtn->setEnabled(false);
    } else {
        setStatusMsg(QString("Step %1 of %2 applied.")
                     .arg(m_stepIndex).arg(m_solution.size()));
    }
}

void CoinRemovalWindow::onReset()
{
    m_currentCoins  = m_initialCoins;
    m_solution.clear();
    m_stepIndex     = 0;
    m_solutionReady = false;
    refreshCoinWidgets();
    m_currentMoveLabel->setText("Reset to initial state.");
    m_currentMoveLabel->setStyleSheet(
        "font-size:15px; font-weight:bold; color:#f6d860; "
        "padding:6px; border-radius:5px; background:#25253a;");
    m_stepBtn->setEnabled(true);
    setStatusMsg("Reset.");
}

void CoinRemovalWindow::onClear()
{
    m_logEdit->clear();
    onReset();
    m_currentMoveLabel->setText("Cleared.");
    setStatusMsg("Cleared.");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CoinRemovalWindow  –  helpers
// ═══════════════════════════════════════════════════════════════════════════════

void CoinRemovalWindow::loadTestCase(int index)
{
    if (index < 0 || index >= static_cast<int>(m_testCases.size())) return;

    const TestCase &tc = m_testCases[index];
    m_initialCoins     = tc.coins;
    m_currentCoins     = tc.coins;
    m_solution.clear();
    m_stepIndex        = 0;
    m_solutionReady    = false;

    // Count heads
    int heads = 0;
    for (char c : tc.coins) if (c == 'H') ++heads;

    m_headsLabel->setText(QString("Heads: %1").arg(heads));
    m_headsLabel->setStyleSheet(
        QString("font-weight:bold; color:%1; padding:2px 6px;")
        .arg(heads % 2 == 0 ? "#ff8080" : "#80ff80"));

    const bool solvable = (heads % 2 != 0);
    m_solvableLabel->setText(solvable ? "Solvable ✔" : "Not solvable ✘");
    m_solvableLabel->setStyleSheet(
        QString("font-weight:bold; color:%1; padding:2px 6px;")
        .arg(solvable ? "#60ff90" : "#ff6060"));

    // Build initial-state string for description
    QString stateStr;
    for (char c : tc.coins) {
        stateStr += '[';
        stateStr += c;
        stateStr += "] ";
    }
    m_caseDescLabel->setText("Initial: " + stateStr.trimmed());

    refreshCoinWidgets();
    m_logEdit->clear();
    m_currentMoveLabel->setText("Select a test case and press Solve or Step.");
    m_currentMoveLabel->setStyleSheet(
        "font-size:15px; font-weight:bold; color:#f6d860; "
        "padding:6px; border-radius:5px; background:#25253a;");
    m_stepBtn->setEnabled(true);
    setStatusMsg(QString("Loaded: %1").arg(QString::fromStdString(tc.description)));
}

void CoinRemovalWindow::refreshCoinWidgets()
{
    // Remove old coin widgets (leave the trailing stretch); guard against null
    while (m_coinLayout->count() > 1) {
        QWidget *w = m_coinLayout->takeAt(0)->widget();
        delete w;
    }
    m_coinWidgets.clear();

    for (int i = 0; i < static_cast<int>(m_currentCoins.size()); ++i) {
        auto *cw = new CoinWidget(m_coinContainer);
        // Insert before the trailing stretch
        m_coinLayout->insertWidget(i, cw);
        m_coinWidgets.push_back(cw);
    }

    // Set states without triggering flash on initial load
    for (int i = 0; i < static_cast<int>(m_currentCoins.size()); ++i)
        m_coinWidgets[i]->setState(m_currentCoins[i]);
}

void CoinRemovalWindow::applyMove(int coinIndex)
{
    if (coinIndex < 0 || coinIndex >= static_cast<int>(m_currentCoins.size())) return;
    if (m_currentCoins[coinIndex] != 'H') return;

    m_currentCoins[coinIndex] = '_';
    if (coinIndex > 0 && m_currentCoins[coinIndex-1] != '_')
        flip(m_currentCoins[coinIndex-1]);
    if (coinIndex < static_cast<int>(m_currentCoins.size())-1
        && m_currentCoins[coinIndex+1] != '_')
        flip(m_currentCoins[coinIndex+1]);

    // Update widgets
    for (int i = 0; i < static_cast<int>(m_coinWidgets.size()); ++i)
        m_coinWidgets[i]->setState(m_currentCoins[i]);
}

void CoinRemovalWindow::appendLog(const QString &text)
{
    m_logEdit->append(text);
    m_logEdit->ensureCursorVisible();
}

void CoinRemovalWindow::setStatusMsg(const QString &text, bool isError)
{
    m_statusBar->setStyleSheet(isError ? "color:#ff8080;" : "color:#8090b0;");
    m_statusBar->showMessage(text);
}
