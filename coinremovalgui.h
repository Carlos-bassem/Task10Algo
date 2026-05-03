#ifndef COINREMOVALGUI_H
#define COINREMOVALGUI_H

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QStatusBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <vector>
#include <set>
#include <string>

// ── CoinWidget ────────────────────────────────────────────────────────────────
// Draws one coin as a circle labelled H (green), T (red), or _ (grey).
// A brief flash animation plays whenever the coin state changes.
class CoinWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal flashOpacity READ flashOpacity WRITE setFlashOpacity)

public:
    explicit CoinWidget(QWidget *parent = nullptr);

    void setState(char state);  // 'H', 'T', or '_'
    char state() const { return m_state; }

    // Animation property
    qreal flashOpacity() const  { return m_flashOpacity; }
    void  setFlashOpacity(qreal v);

    QSize sizeHint() const override { return QSize(60, 60); }
    QSize minimumSizeHint() const override { return QSize(48, 48); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    char   m_state        = '_';
    qreal  m_flashOpacity = 0.0;

    void startFlash();
};

// ── CoinRemovalWindow ─────────────────────────────────────────────────────────
// Main application window.
class CoinRemovalWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CoinRemovalWindow(QWidget *parent = nullptr);

private slots:
    void onTestCaseChanged(int index);
    void onSolve();
    void onStep();
    void onReset();
    void onClear();

private:
    // ── Algorithm helpers ────────────────────────────────────────────────────
    static std::string stateToString(const std::vector<char>& coins);
    static bool        isSolved(const std::vector<char>& coins);
    static bool        hasHeads(const std::vector<char>& coins);
    static void        flip(char& coin);
    static bool        solve(std::vector<char>& coins,
                             std::set<std::string>& visited,
                             std::vector<int>& moves);

    // Returns true and fills solution; returns false when no solution exists.
    bool solveCoinRemoval(std::vector<char> coins, std::vector<int>& solution);

    // ── UI helpers ───────────────────────────────────────────────────────────
    void buildUi();
    void loadTestCase(int index);
    void refreshCoinWidgets();
    void applyMove(int coinIndex);
    void appendLog(const QString& text);
    void setStatusMsg(const QString& text, bool isError = false);

    // ── Test-case data ───────────────────────────────────────────────────────
    struct TestCase {
        std::string        description;
        std::vector<char>  coins;
    };
    static std::vector<TestCase> buildTestCases();

    // ── Member variables ─────────────────────────────────────────────────────
    std::vector<TestCase>  m_testCases;
    std::vector<char>      m_initialCoins;   // snapshot for Reset
    std::vector<char>      m_currentCoins;   // live state
    std::vector<int>       m_solution;       // moves returned by solver
    int                    m_stepIndex = 0;  // next move to apply in Step mode
    bool                   m_solutionReady = false;

    // ── Widgets ──────────────────────────────────────────────────────────────
    QComboBox             *m_caseCombo       = nullptr;
    QLabel                *m_caseDescLabel   = nullptr;
    QLabel                *m_headsLabel      = nullptr;
    QLabel                *m_solvableLabel   = nullptr;

    QWidget               *m_coinContainer   = nullptr;
    QHBoxLayout           *m_coinLayout      = nullptr;
    std::vector<CoinWidget*> m_coinWidgets;

    QPushButton           *m_solveBtn        = nullptr;
    QPushButton           *m_stepBtn         = nullptr;
    QPushButton           *m_resetBtn        = nullptr;
    QPushButton           *m_clearBtn        = nullptr;

    QLabel                *m_currentMoveLabel= nullptr;
    QTextEdit             *m_logEdit         = nullptr;
    QStatusBar            *m_statusBar       = nullptr;
};

#endif // COINREMOVALGUI_H
