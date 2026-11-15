#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QTextEdit>
#include <QRandomGenerator>
#include <QSizePolicy>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QVector>
#include <QRadialGradient>
#include <QStringList>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

#include "collision.h"
#include "graph.h"
#include "routing.hpp"

struct SpaceObject {
    QString name;
    QString type;       // "Satellite" or "Debris"
    double  x;          // demo-plane coordinates used by the collision module
    double  y;
    double  altitude;   // illustrative, used only for drawing
    double  phase;      // illustrative, used only for drawing
    double  risk = 1.0;                                       // derived from nearest distance
    double  nearestDist = std::numeric_limits<double>::infinity();
    QString nearestName;
};

static QString appStyle()
{
    return QStringLiteral(R"(
        QWidget { background: #0b1220; color: #e6edf7; font-family: "Segoe UI"; font-size: 10pt; }
        QMainWindow { background: #0b1220; }
        QFrame#Sidebar { background: #080e19; border-right: 1px solid #1e293b; }
        QFrame#Topbar { background: #0f1728; border-bottom: 1px solid #1e293b; }
        QLabel#Brand { color: #f8fafc; font-size: 19pt; font-weight: 700; }
        QLabel#Muted { color: #8da1bd; }
        QLabel#PageTitle { font-size: 22pt; font-weight: 700; color: #f8fafc; }
        QLabel#CardTitle { color: #91a4c0; font-size: 9pt; }
        QLabel#Metric { color: #f8fafc; font-size: 23pt; font-weight: 700; }
        QFrame#Panel, QWidget#Panel { background: #111c2e; border: 1px solid #22324a; border-radius: 12px; }
        QPushButton { background: #17243a; border: 1px solid #2b3d58; border-radius: 8px; padding: 9px 12px; color: #dce8f8; text-align: left; }
        QPushButton:hover { background: #20314b; border-color: #3b82f6; }
        QPushButton#NavButton { background: transparent; border: 0; padding: 11px 12px; color: #9fb0c8; text-align: left; }
        QPushButton#NavButton:checked, QPushButton#NavButton:hover { background: #142641; color: #67d7ff; border-left: 3px solid #38bdf8; }
        QPushButton#Primary { background: #087ea4; border: 1px solid #0891b2; color: white; font-weight: 600; text-align: center; }
        QPushButton#Primary:hover { background: #0891b2; }
        QTableWidget { background: #0e192a; alternate-background-color: #132138; gridline-color: #24354d; border: 1px solid #263750; border-radius: 8px; selection-background-color: #164e63; }
        QHeaderView::section { background: #17243a; color: #a9bdd7; border: 0; padding: 9px; font-weight: 600; }
        QTableWidget::item { padding: 6px; }
        QLineEdit, QComboBox, QDoubleSpinBox, QTextEdit { background: #0b1525; border: 1px solid #2b3d58; border-radius: 7px; padding: 8px; }
        QProgressBar { background: #0b1525; border: 0; border-radius: 4px; height: 8px; text-align: center; }
        QProgressBar::chunk { background: #22d3ee; border-radius: 4px; }
        QScrollArea { border: none; }
    )");
}

class OrbitWidget : public QWidget
{
public:
    explicit OrbitWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(420, 340);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setObjects(const QVector<SpaceObject> *objects) { objects_ = objects; update(); }
    void setRunning(bool running) { running_ = running; update(); }
    void advance() { if (running_) { angle_ += 0.012; update(); } }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#0b1525"));

        // Subtle star field
        for (int i = 0; i < 75; ++i) {
            const int x = (i * 97 + 31) % qMax(1, width());
            const int y = (i * 53 + 19) % qMax(1, height());
            p.setPen(QPen(QColor(116, 158, 202, (i % 3 + 1) * 35), (i % 4 == 0) ? 2 : 1));
            p.drawPoint(x, y);
        }

        const QPointF c(width() * 0.5, height() * 0.53);
        const qreal earthR = qMin(width(), height()) * 0.19;
        for (int i = 0; i < 3; ++i) {
            p.setPen(QPen(QColor(55, 157, 205, 80 - i * 18), 1));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(c, earthR + 35 + i * 37, earthR + 16 + i * 27);
        }

        QRadialGradient earth(c, earthR * 1.2);
        earth.setColorAt(0.0, QColor("#54c9e8"));
        earth.setColorAt(0.55, QColor("#1675a8"));
        earth.setColorAt(1.0, QColor("#10294c"));
        p.setPen(QPen(QColor("#4dd8f5"), 1.2));
        p.setBrush(earth);
        p.drawEllipse(c, earthR, earthR);

        // Stylized continent shapes
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(72, 190, 151, 180));
        p.drawEllipse(QRectF(c.x() - earthR * .55, c.y() - earthR * .52, earthR * .55, earthR * .32));
        p.drawEllipse(QRectF(c.x() + earthR * .05, c.y() - earthR * .12, earthR * .43, earthR * .62));
        p.drawEllipse(QRectF(c.x() - earthR * .4, c.y() + earthR * .25, earthR * .26, earthR * .27));

        if (objects_) {
            for (int i = 0; i < objects_->size(); ++i) {
                const SpaceObject &o = objects_->at(i);
                const qreal rx = earthR + 35 + (i % 3) * 37;
                const qreal ry = earthR + 16 + (i % 3) * 27;
                const qreal a = o.phase + angle_ * (o.type == "Debris" ? 1.25 : 0.75);
                const QPointF pt(c.x() + std::cos(a) * rx, c.y() + std::sin(a) * ry);
                const QColor col = o.type == "Debris" ? QColor("#fb7185") : QColor("#67e8f9");
                p.setPen(QPen(col, 1));
                p.setBrush(col);
                p.drawEllipse(pt, o.type == "Debris" ? 4.0 : 5.5, o.type == "Debris" ? 4.0 : 5.5);
                if (i < 5) {
                    p.setPen(QColor("#c8d8ec"));
                    p.drawText(pt + QPointF(8, -7), o.name);
                }
            }
        }

        p.setPen(QColor("#8da1bd"));
        p.drawText(16, 25, "ORBITAL VIEW  /  SIMULATED OBJECTS");
        p.setPen(QColor("#4bd8f2"));
        p.drawText(16, height() - 16, running_ ? "SIMULATION RUNNING" : "SIMULATION PAUSED");
    }

private:
    const QVector<SpaceObject> *objects_ = nullptr;
    qreal angle_ = 0.0;
    bool running_ = false;
};

static QFrame *makePanel(const QString &title, QVBoxLayout *&layoutOut)
{
    auto *panel = new QFrame;
    panel->setObjectName("Panel");
    layoutOut = new QVBoxLayout(panel);
    layoutOut->setContentsMargins(16, 14, 16, 16);
    layoutOut->setSpacing(10);
    auto *heading = new QLabel(title);
    heading->setStyleSheet("font-weight: 700; color: #eaf2ff; font-size: 11pt;");
    layoutOut->addWidget(heading);
    return panel;
}

static QFrame *metricCard(const QString &title, const QString &value, const QString &caption,
                          QLabel **valueOut = nullptr, QLabel **captionOut = nullptr)
{
    auto *card = new QFrame;
    card->setObjectName("Panel");
    auto *l = new QVBoxLayout(card);
    l->setContentsMargins(16, 14, 16, 14);
    l->setSpacing(5);
    auto *t = new QLabel(title);
    t->setObjectName("CardTitle");
    auto *v = new QLabel(value);
    v->setObjectName("Metric");
    auto *c = new QLabel(caption);
    c->setObjectName("Muted");
    l->addWidget(t);
    l->addWidget(v);
    l->addWidget(c);
    l->addStretch();
    if (valueOut) *valueOut = v;
    if (captionOut) *captionOut = c;
    return card;
}

static void clearLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        if (QLayout *child = item->layout()) clearLayout(child);
        delete item;
    }
}

static QVector<SpaceObject> demoObjects()
{
    //            name          type         x     y    alt  phase
    return {
        {"ORION-01",   "Satellite",  10,  10, 550, 0.3},
        {"ASTRA-EYE",  "Satellite",  60,  40, 620, 1.7},
        {"DEBRIS-17",  "Debris",     15,  16, 570, 2.8},
        {"NAVSTAR-04", "Satellite", 120,  90, 720, 4.1},
        {"FRAG-203",   "Debris",     68,  48, 610, 5.2},
        {"COMMS-12",   "Satellite",  30,  20, 680, 0.9}
    };
}

static QColor levelColor(const QString &level)
{
    if (level == "CRITICAL") return QColor("#fb7185");
    if (level == "WARNING")  return QColor("#fbbf24");
    return QColor("#6ee7b7");
}

static QString clockText()
{
    const QDateTime local = QDateTime::currentDateTime();
    const int off = local.offsetFromUtc();            // seconds east of UTC
    const int a = qAbs(off);
    return QString("LOCAL  %1  (UTC%2%3:%4)    |    UTC  %5")
        .arg(local.toString("yyyy-MM-dd  HH:mm:ss"))
        .arg(off < 0 ? QString("-") : QString("+"))
        .arg(a / 3600, 2, 10, QChar('0'))
        .arg((a % 3600) / 60, 2, 10, QChar('0'))
        .arg(QDateTime::currentDateTimeUtc().toString("HH:mm:ss"));
}

class MainWindow : public QMainWindow
{
public:
    MainWindow()
        : network_(new Graph),
          sectors_(astra::buildSampleSectorGraph())
    {
        setWindowTitle("ASTRA | Space Traffic & Risk Avoidance");
        resize(1440, 900);
        setMinimumSize(1080, 700);
        setStyleSheet(appStyle());

        objects_ = demoObjects();
        pageNames_ = {"Mission Overview", "Orbit Monitor", "Collision Risk", "Space Network",
                      "Maneuver Planner", "Analytics", "Learn ASTRA"};

        auto *root = new QWidget;
        auto *rootLayout = new QHBoxLayout(root);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        auto *sidebar = new QFrame;
        sidebar->setObjectName("Sidebar");
        sidebar->setFixedWidth(225);
        auto *side = new QVBoxLayout(sidebar);
        side->setContentsMargins(14, 20, 14, 16);
        side->setSpacing(8);

        auto *brand = new QLabel("✦ ASTRA");
        brand->setObjectName("Brand");
        side->addWidget(brand);
        auto *sub = new QLabel("SPACE TRAFFIC INTELLIGENCE");
        sub->setObjectName("Muted");
        sub->setStyleSheet("font-size: 8pt; letter-spacing: 1px;");
        side->addWidget(sub);
        side->addSpacing(24);
        auto *navLabel = new QLabel("WORKSPACE");
        navLabel->setObjectName("Muted");
        navLabel->setStyleSheet("font-size: 8pt; font-weight: 700;");
        side->addWidget(navLabel);

        stack_ = new QStackedWidget;
        const QStringList symbols = {"⌂", "◎", "⚠", "⌬", "➤", "▥", "ⓘ"};
        for (int i = 0; i < pageNames_.size(); ++i) {
            auto *b = new QPushButton(symbols[i] + "   " + pageNames_[i]);
            b->setObjectName("NavButton");
            b->setCheckable(true);
            b->setChecked(i == 0);
            side->addWidget(b);
            navButtons_.append(b);
            connect(b, &QPushButton::clicked, this, [this, i]() { showPage(i); });
        }
        side->addStretch();
        auto *status = new QLabel("●  SYSTEM ONLINE\n\nCore modules linked\nDemo coordinates only");
        status->setStyleSheet("color: #7dd3a7; background: #10251f; border: 1px solid #1e4a3a; border-radius: 8px; padding: 12px;");
        side->addWidget(status);

        auto *right = new QWidget;
        auto *rightLayout = new QVBoxLayout(right);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setSpacing(0);

        auto *top = new QFrame;
        top->setObjectName("Topbar");
        auto *topLayout = new QHBoxLayout(top);
        topLayout->setContentsMargins(22, 12, 22, 12);
        auto *crumb = new QLabel("ASTRA  /  <span style='color:#eaf2ff'>Mission Overview</span>");
        crumb->setTextFormat(Qt::RichText);
        crumb->setObjectName("Muted");
        pageCrumb_ = crumb;
        topLayout->addWidget(crumb);
        topLayout->addStretch();
        auto *utc = new QLabel;
        utc->setObjectName("Muted");
        topLayout->addWidget(utc);
        auto *utcTimer = new QTimer(this);
        connect(utcTimer, &QTimer::timeout, this, [utc]() {
            utc->setText(clockText());
        });
        utcTimer->start(1000);
        utc->setText(clockText());
        rightLayout->addWidget(top);

        computeRisks();
        stack_->addWidget(makeOverview());
        stack_->addWidget(makeOrbitPage());
        stack_->addWidget(makeRiskPage());
        stack_->addWidget(makeNetworkPage());
        stack_->addWidget(makeManeuverPage());
        stack_->addWidget(makeAnalyticsPage());
        stack_->addWidget(makeLearnPage());
        rightLayout->addWidget(stack_, 1);

        rootLayout->addWidget(sidebar);
        rootLayout->addWidget(right, 1);
        setCentralWidget(root);

        auto *orbitTimer = new QTimer(this);
        connect(orbitTimer, &QTimer::timeout, this, [this]() {
            if (orbit_) orbit_->advance();
            if (orbitView_) orbitView_->advance();
        });
        orbitTimer->start(40);

        refreshAll();
    }

private:
    // ---- data ----
    QVector<SpaceObject> objects_;
    std::unique_ptr<Graph> network_;      // C graph module (heap: it is ~120 KB)
    astra::RouteGraph sectors_;           // C++ Dijkstra module
    QStringList pageNames_;
    bool running_ = true;

    // ---- widgets ----
    QStackedWidget *stack_ = nullptr;
    QLabel *pageCrumb_ = nullptr;
    QVector<QPushButton *> navButtons_;
    OrbitWidget *orbit_ = nullptr;
    OrbitWidget *orbitView_ = nullptr;
    QLabel *simulationState_ = nullptr;
    QPushButton *simToggle_ = nullptr;

    QLabel *ovTracked_ = nullptr, *ovAlerts_ = nullptr, *ovAlertsCap_ = nullptr;
    QLabel *ovLinks_ = nullptr, *ovLinksCap_ = nullptr;

    QTableWidget *riskTable_ = nullptr;
    QTableWidget *alertTable_ = nullptr;
    QLabel *alertSummary_ = nullptr;

    QComboBox *netStart_ = nullptr, *netEnd_ = nullptr;
    QDoubleSpinBox *netRange_ = nullptr;
    QTextEdit *netOutput_ = nullptr;

    QComboBox *maneuverTarget_ = nullptr;
    QComboBox *sectorStart_ = nullptr, *sectorEnd_ = nullptr;
    QTextEdit *routeOutput_ = nullptr;

    QLabel *anCatalogue_ = nullptr, *anDebris_ = nullptr, *anSats_ = nullptr, *anMean_ = nullptr;
    QVBoxLayout *barsLayout_ = nullptr;

    // ---- navigation ----
    void showPage(int index)
    {
        stack_->setCurrentIndex(index);
        for (int i = 0; i < navButtons_.size(); ++i) navButtons_[i]->setChecked(i == index);
        pageCrumb_->setText("ASTRA  /  <span style='color:#eaf2ff'>" + pageNames_.value(index) + "</span>");
        applyRunning();
    }

    void applyRunning()
    {
        if (orbit_) orbit_->setRunning(running_);
        if (orbitView_) orbitView_->setRunning(running_);
        if (simToggle_) simToggle_->setText(running_ ? "Pause simulation" : "Start simulation");
        if (simulationState_) simulationState_->setText(running_ ? "RUNNING" : "PAUSED");
    }

    QWidget *pageBase(const QString &title, const QString &subtitle, QVBoxLayout **layoutOut)
    {
        auto *page = new QWidget;
        auto *outer = new QVBoxLayout(page);
        outer->setContentsMargins(24, 22, 24, 24);
        outer->setSpacing(16);
        auto *titleLabel = new QLabel(title);
        titleLabel->setObjectName("PageTitle");
        auto *subLabel = new QLabel(subtitle);
        subLabel->setObjectName("Muted");
        outer->addWidget(titleLabel);
        outer->addWidget(subLabel);
        *layoutOut = outer;
        return page;
    }

    // ---- core-module glue ----

    // Collision module: pairwise distances -> nearest neighbour + heuristic score.
    void computeRisks()
    {
        const int n = objects_.size();
        for (int i = 0; i < n; ++i) {
            SpaceObject &a = objects_[i];
            a.nearestDist = std::numeric_limits<double>::infinity();
            a.nearestName.clear();
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                const double d = calculateDistance(a.x, a.y, objects_[j].x, objects_[j].y);
                if (d < a.nearestDist) { a.nearestDist = d; a.nearestName = objects_[j].name; }
            }
            a.risk = std::isfinite(a.nearestDist)
                ? qBound(1.0, 100.0 * (1.0 - a.nearestDist / (2.0 * ASTRA_WARNING_DISTANCE)), 99.0)
                : 1.0;
        }
    }

    struct Pair { int a, b; double dist; };

    QVector<Pair> alertPairs() const
    {
        QVector<Pair> out;
        for (int i = 0; i < objects_.size(); ++i)
            for (int j = i + 1; j < objects_.size(); ++j) {
                const double d = calculateDistance(objects_[i].x, objects_[i].y, objects_[j].x, objects_[j].y);
                if (d <= ASTRA_WARNING_DISTANCE) out.append({i, j, d});
            }
        std::sort(out.begin(), out.end(), [](const Pair &p, const Pair &q) { return p.dist < q.dist; });
        return out;
    }

    // Graph module: objects are nodes, links exist within the chosen range.
    void rebuildNetwork()
    {
        const int n = qMin<int>(objects_.size(), MAX_NODES);
        initializeGraph(network_.get(), n);
        const double range = netRange_ ? netRange_->value() : 60.0;
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) {
                const double d = calculateDistance(objects_[i].x, objects_[i].y, objects_[j].x, objects_[j].y);
                if (d <= range) addUndirectedEdge(network_.get(), i, j, d);
            }
    }

    int linkCount() const
    {
        int links = 0;
        for (int i = 0; i < network_->nodeCount; ++i)
            for (int j = i + 1; j < network_->nodeCount; ++j)
                if (hasEdge(network_.get(), i, j)) ++links;
        return links;
    }

    void fillObjectCombo(QComboBox *box)
    {
        if (!box) return;
        const int keep = box->currentIndex();
        box->blockSignals(true);
        box->clear();
        for (const auto &o : objects_) box->addItem(o.name);
        if (keep >= 0 && keep < box->count()) box->setCurrentIndex(keep);
        box->blockSignals(false);
    }

    void refreshAll()
    {
        computeRisks();
        rebuildNetwork();
        fillObjectCombo(netStart_);
        fillObjectCombo(netEnd_);
        if (netEnd_ && netEnd_->currentIndex() == 0 && netEnd_->count() > 1 && !netEndTouched_)
            netEnd_->setCurrentIndex(netEnd_->count() - 1);
        fillObjectCombo(maneuverTarget_);

        updateRiskTables();
        updateOverviewMetrics();
        updateAnalytics();
        runNetworkAnalysis();

        if (orbit_) orbit_->update();
        if (orbitView_) orbitView_->update();
    }
    bool netEndTouched_ = false;

    void updateOverviewMetrics()
    {
        const auto pairs = alertPairs();
        int critical = 0;
        for (const auto &p : pairs) if (p.dist <= ASTRA_CRITICAL_DISTANCE) ++critical;
        if (ovTracked_) ovTracked_->setText(QString::number(objects_.size()));
        if (ovAlerts_) ovAlerts_->setText(QString("%1").arg(static_cast<int>(pairs.size()), 2, 10, QChar('0')));
        if (ovAlertsCap_) ovAlertsCap_->setText(QString("%1 critical, %2 warning").arg(critical).arg(static_cast<int>(pairs.size()) - critical));
        if (ovLinks_) ovLinks_->setText(QString("%1").arg(linkCount(), 2, 10, QChar('0')));
        if (ovLinksCap_) ovLinksCap_->setText(QString("Range %1 units").arg(netRange_ ? netRange_->value() : 60.0, 0, 'f', 0));
    }

    void updateRiskTables()
    {
        if (riskTable_) {
            riskTable_->setRowCount(objects_.size());
            for (int i = 0; i < objects_.size(); ++i) {
                const auto &o = objects_[i];
                const bool hasNeighbour = std::isfinite(o.nearestDist);
                const QString level = hasNeighbour ? QString::fromLatin1(getRiskLevel(o.nearestDist)) : QString("SAFE");
                const QStringList values = {
                    o.name, o.type,
                    hasNeighbour ? o.nearestName : QString("—"),
                    hasNeighbour ? QString::number(o.nearestDist, 'f', 2) : QString("—"),
                    QString::number(static_cast<int>(o.risk)),
                    level
                };
                for (int col = 0; col < values.size(); ++col) {
                    auto *item = new QTableWidgetItem(values[col]);
                    if (col == 5) item->setForeground(levelColor(level));
                    riskTable_->setItem(i, col, item);
                }
            }
        }

        const auto pairs = alertPairs();
        if (alertTable_) {
            alertTable_->setRowCount(pairs.size());
            for (int r = 0; r < pairs.size(); ++r) {
                const auto &p = pairs[r];
                const QString level = QString::fromLatin1(getRiskLevel(p.dist));
                const QStringList values = {objects_[p.a].name, objects_[p.b].name,
                                            QString::number(p.dist, 'f', 2), level};
                for (int col = 0; col < values.size(); ++col) {
                    auto *item = new QTableWidgetItem(values[col]);
                    if (col == 3) item->setForeground(levelColor(level));
                    alertTable_->setItem(r, col, item);
                }
            }
        }
        if (alertSummary_) {
            int critical = 0;
            for (const auto &p : pairs) if (p.dist <= ASTRA_CRITICAL_DISTANCE) ++critical;
            QString text = QString("Alerts: %1   |   Critical: %2   |   Warning: %3")
                               .arg(static_cast<int>(pairs.size())).arg(critical).arg(static_cast<int>(pairs.size()) - critical);
            if (pairs.isEmpty()) text += "   —   no proximity alerts detected";
            else text += QString("   |   Closest pair: %1 / %2 (%3)")
                             .arg(objects_[pairs[0].a].name, objects_[pairs[0].b].name)
                             .arg(pairs[0].dist, 0, 'f', 2);
            alertSummary_->setText(text);
        }
    }

    // ---- pages ----
    QWidget *makeOverview()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Mission Overview", "Live situational awareness for a simulated orbital environment.", &outer);
        auto *metrics = new QGridLayout;
        metrics->setSpacing(12);
        metrics->addWidget(metricCard("TRACKED OBJECTS", QString::number(objects_.size()), "Catalogued satellites + debris", &ovTracked_), 0, 0);
        metrics->addWidget(metricCard("PROXIMITY ALERTS", "00", "—", &ovAlerts_, &ovAlertsCap_), 0, 1);
        metrics->addWidget(metricCard("NETWORK LINKS", "00", "—", &ovLinks_, &ovLinksCap_), 0, 2);
        metrics->addWidget(metricCard("SYSTEM STATUS", "ONLINE", "Prototype simulation"), 0, 3);
        outer->addLayout(metrics);

        auto *row = new QHBoxLayout;
        row->setSpacing(14);
        QVBoxLayout *orbitLayout = nullptr;
        auto *orbitPanel = makePanel("Orbital environment", orbitLayout);
        orbit_ = new OrbitWidget;
        orbit_->setObjects(&objects_);
        orbit_->setRunning(running_);
        orbitLayout->addWidget(orbit_, 1);
        auto *orbitControls = new QHBoxLayout;
        simToggle_ = new QPushButton("Pause simulation");
        simToggle_->setObjectName("Primary");
        connect(simToggle_, &QPushButton::clicked, this, [this]() {
            running_ = !running_;
            applyRunning();
        });
        simulationState_ = new QLabel("RUNNING");
        simulationState_->setStyleSheet("color:#67e8a5; font-weight:700;");
        orbitControls->addWidget(simToggle_);
        orbitControls->addStretch();
        orbitControls->addWidget(simulationState_);
        orbitLayout->addLayout(orbitControls);
        row->addWidget(orbitPanel, 2);

        QVBoxLayout *activityLayout = nullptr;
        auto *activityPanel = makePanel("Mission activity", activityLayout);
        const QStringList activities = {
            "●  Object catalogue initialized",
            "●  Collision module: pairwise screening done",
            "●  Graph module: space network built",
            "●  Routing module: safe-route planner ready"
        };
        for (const QString &a : activities) {
            auto *l = new QLabel(a);
            l->setWordWrap(true);
            l->setStyleSheet("padding: 10px 2px; color:#b8c8de; border-bottom:1px solid #22324a;");
            activityLayout->addWidget(l);
        }
        activityLayout->addStretch();
        auto *hint = new QLabel("Prototype uses illustrative data. Validate all algorithms before operational use.");
        hint->setWordWrap(true);
        hint->setObjectName("Muted");
        activityLayout->addWidget(hint);
        row->addWidget(activityPanel, 1);
        outer->addLayout(row, 1);
        return page;
    }

    QWidget *makeOrbitPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Orbit Monitor", "Inspect the simulated satellite and debris catalogue.", &outer);
        QVBoxLayout *orbitLayout = nullptr;
        auto *panel = makePanel("Orbital visualization", orbitLayout);
        orbitView_ = new OrbitWidget;
        orbitView_->setObjects(&objects_);
        orbitView_->setRunning(running_);
        orbitLayout->addWidget(orbitView_, 1);
        auto *note = new QLabel("Cyan markers represent satellites; coral markers represent debris. The drawing is illustrative: "
                                "marker positions are not computed from the collision-module coordinates and this is not an ephemeris.");
        note->setObjectName("Muted");
        note->setWordWrap(true);
        orbitLayout->addWidget(note);
        outer->addWidget(panel, 1);
        return page;
    }

    QWidget *makeRiskPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Collision Risk",
            "Pairwise distance screening (collision module). Thresholds: warning ≤ 20, critical ≤ 10 demo units.", &outer);

        alertSummary_ = new QLabel;
        alertSummary_->setStyleSheet("color:#eaf2ff; font-weight:700; background:#111c2e; border:1px solid #22324a; border-radius:8px; padding:10px;");
        outer->addWidget(alertSummary_);

        auto *row = new QHBoxLayout;
        row->setSpacing(14);

        QVBoxLayout *registerLayout = nullptr;
        auto *registerPanel = makePanel("Risk register", registerLayout);
        riskTable_ = new QTableWidget(0, 6);
        riskTable_->setHorizontalHeaderLabels({"Object", "Type", "Nearest", "Distance", "Score", "Level"});
        riskTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        riskTable_->verticalHeader()->setVisible(false);
        riskTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        riskTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        registerLayout->addWidget(riskTable_, 1);
        auto *score = new QLabel("Score is a heuristic (100 at distance 0, 0 at 40 units); it is not a collision probability.");
        score->setObjectName("Muted");
        score->setWordWrap(true);
        registerLayout->addWidget(score);
        row->addWidget(registerPanel, 3);

        QVBoxLayout *alertLayout = nullptr;
        auto *alertPanel = makePanel("Proximity alerts", alertLayout);
        alertTable_ = new QTableWidget(0, 4);
        alertTable_->setHorizontalHeaderLabels({"Object 1", "Object 2", "Distance", "Level"});
        alertTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        alertTable_->verticalHeader()->setVisible(false);
        alertTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        alertTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        alertLayout->addWidget(alertTable_, 1);
        row->addWidget(alertPanel, 2);
        outer->addLayout(row, 1);

        auto *buttons = new QHBoxLayout;
        auto *add = new QPushButton("Add object…");
        add->setObjectName("Primary");
        connect(add, &QPushButton::clicked, this, [this]() { addObjectDialog(); });
        auto *reset = new QPushButton("Reset demo data");
        connect(reset, &QPushButton::clicked, this, [this]() {
            objects_ = demoObjects();
            refreshAll();
        });
        buttons->addWidget(add);
        buttons->addWidget(reset);
        buttons->addStretch();
        outer->addLayout(buttons);
        return page;
    }

    void addObjectDialog()
    {
        if (objects_.size() >= 50) {
            QMessageBox::warning(this, "ASTRA", "The catalogue is limited to 50 objects.");
            return;
        }
        QDialog dlg(this);
        dlg.setWindowTitle("Add space object");
        auto *form = new QFormLayout(&dlg);
        auto *name = new QLineEdit;
        auto *type = new QComboBox;
        type->addItems({"Satellite", "Debris"});
        auto *x = new QDoubleSpinBox;
        auto *y = new QDoubleSpinBox;
        for (auto *s : {x, y}) { s->setRange(-1000.0, 1000.0); s->setDecimals(2); }
        form->addRow("Name", name);
        form->addRow("Type", type);
        form->addRow("X coordinate", x);
        form->addRow("Y coordinate", y);
        auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        form->addRow(box);
        connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        if (dlg.exec() != QDialog::Accepted) return;

        const QString n = name->text().trimmed();
        if (n.isEmpty()) {
            QMessageBox::warning(this, "ASTRA", "Please enter a name.");
            return;
        }
        for (const auto &o : objects_) {
            if (o.name.compare(n, Qt::CaseInsensitive) == 0) {
                QMessageBox::warning(this, "ASTRA", "An object with that name already exists.");
                return;
            }
        }
        const int k = objects_.size();
        SpaceObject obj;
        obj.name = n;
        obj.type = type->currentText();
        obj.x = x->value();
        obj.y = y->value();
        obj.altitude = 550.0 + 40.0 * (k % 5);
        obj.phase = 0.7 * k;
        objects_.append(obj);
        refreshAll();
    }

    QWidget *makeNetworkPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Space Network",
            "Objects are graph nodes; objects within link range are connected (graph module: BFS, DFS, route finding).", &outer);
        auto *row = new QHBoxLayout;
        row->setSpacing(14);

        QVBoxLayout *ctlLayout = nullptr;
        auto *ctlPanel = makePanel("Network settings", ctlLayout);
        auto *form = new QFormLayout;
        netRange_ = new QDoubleSpinBox;
        netRange_->setRange(1.0, 500.0);
        netRange_->setValue(60.0);
        netRange_->setSuffix(" units");
        netStart_ = new QComboBox;
        netEnd_ = new QComboBox;
        for (const auto &o : objects_) { netStart_->addItem(o.name); netEnd_->addItem(o.name); }
        if (netEnd_->count() > 1) netEnd_->setCurrentIndex(netEnd_->count() - 1);
        form->addRow("Link range", netRange_);
        form->addRow("Start object", netStart_);
        form->addRow("Destination", netEnd_);
        ctlLayout->addLayout(form);
        auto *hint = new QLabel("Objects outside every other object's range stay isolated, so no route can reach them.");
        hint->setObjectName("Muted");
        hint->setWordWrap(true);
        ctlLayout->addWidget(hint);
        ctlLayout->addStretch();
        row->addWidget(ctlPanel, 1);

        QVBoxLayout *outLayout = nullptr;
        auto *outPanel = makePanel("Network analysis", outLayout);
        netOutput_ = new QTextEdit;
        netOutput_->setReadOnly(true);
        outLayout->addWidget(netOutput_, 1);
        row->addWidget(outPanel, 2);
        outer->addLayout(row, 1);

        connect(netRange_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
            rebuildNetwork();
            updateOverviewMetrics();
            runNetworkAnalysis();
        });
        connect(netStart_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { runNetworkAnalysis(); });
        connect(netEnd_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
            netEndTouched_ = true;
            runNetworkAnalysis();
        });
        return page;
    }

    QString orderToText(const int *order, int count) const
    {
        QStringList parts;
        for (int i = 0; i < count; ++i) parts << objects_[order[i]].name;
        return parts.join("  →  ");
    }

    void runNetworkAnalysis()
    {
        if (!netOutput_ || !netStart_ || !netEnd_) return;
        const int s = netStart_->currentIndex();
        const int d = netEnd_->currentIndex();
        const int n = network_->nodeCount;
        if (s < 0 || d < 0 || s >= n || d >= n) { netOutput_->setPlainText("No objects in the catalogue."); return; }

        QString t;
        t += QString("NETWORK  —  %1 nodes, %2 links (range %3)\n\nAdjacency\n")
                 .arg(n).arg(linkCount()).arg(netRange_->value(), 0, 'f', 1);
        for (int i = 0; i < n; ++i) {
            QStringList links;
            for (int j = 0; j < n; ++j)
                if (hasEdge(network_.get(), i, j))
                    links << QString("%1 (%2)").arg(objects_[j].name).arg(network_->weights[i][j], 0, 'f', 1);
            t += "  " + objects_[i].name + ":  " + (links.isEmpty() ? QString("isolated") : links.join(", ")) + "\n";
        }

        int order[MAX_NODES];
        int cnt = graphBFSOrder(network_.get(), s, order);
        t += "\nBFS from " + objects_[s].name + "\n  " + orderToText(order, cnt) + "\n";
        cnt = graphDFSOrder(network_.get(), s, order);
        t += "\nDFS from " + objects_[s].name + "\n  " + orderToText(order, cnt) + "\n";

        int path[MAX_NODES];
        const int len = graphFindPath(network_.get(), s, d, path);
        t += "\nRoute " + objects_[s].name + " → " + objects_[d].name + " (fewest hops)\n";
        if (len == 0) {
            t += "  No route exists at the current link range.\n";
        } else {
            t += "  " + orderToText(path, len) + "\n";
            t += QString("  Hops: %1   Total link distance: %2\n").arg(len - 1).arg(graphPathWeight(network_.get(), path, len), 0, 'f', 2);
        }
        netOutput_->setPlainText(t);
    }

    QWidget *makeManeuverPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Maneuver Planner",
            "Illustrative proposal for a selected object, plus a safe-route planner (Dijkstra).", &outer);
        auto *row = new QHBoxLayout;
        row->setSpacing(14);

        // --- left column: proposal + route planner settings ---
        auto *left = new QVBoxLayout;
        QVBoxLayout *formLayout = nullptr;
        auto *formPanel = makePanel("Proposal settings", formLayout);
        auto *form = new QFormLayout;
        maneuverTarget_ = new QComboBox;
        for (const auto &o : objects_) maneuverTarget_->addItem(o.name);
        auto *strategy = new QComboBox;
        strategy->addItems({"Small along-track adjustment", "Radial adjustment", "Review only — no maneuver"});
        auto *horizon = new QComboBox;
        horizon->addItems({"Next 6 hours", "Next 24 hours", "Next 72 hours"});
        form->addRow("Target object", maneuverTarget_);
        form->addRow("Strategy", strategy);
        form->addRow("Planning horizon", horizon);
        formLayout->addLayout(form);
        auto *generate = new QPushButton("Generate demo proposal");
        generate->setObjectName("Primary");
        formLayout->addWidget(generate);
        left->addWidget(formPanel);

        QVBoxLayout *routeLayout = nullptr;
        auto *routePanel = makePanel("Safe route planner (sectors)", routeLayout);
        auto *routeForm = new QFormLayout;
        sectorStart_ = new QComboBox;
        sectorEnd_ = new QComboBox;
        for (int i = 0; i < sectors_.nodeCount(); ++i) {
            sectorStart_->addItem(QString("Sector %1").arg(i));
            sectorEnd_->addItem(QString("Sector %1").arg(i));
        }
        sectorEnd_->setCurrentIndex(sectors_.nodeCount() - 1);
        routeForm->addRow("From", sectorStart_);
        routeForm->addRow("To", sectorEnd_);
        routeLayout->addLayout(routeForm);
        auto *findRouteBtn = new QPushButton("Find lowest-cost route");
        findRouteBtn->setObjectName("Primary");
        routeLayout->addWidget(findRouteBtn);
        auto *costNote = new QLabel("Cost per leg = distance × (1 + risk factor) + fuel cost.");
        costNote->setObjectName("Muted");
        costNote->setWordWrap(true);
        routeLayout->addWidget(costNote);
        left->addWidget(routePanel);
        left->addStretch();
        row->addLayout(left, 1);

        // --- right column: outputs ---
        auto *rightCol = new QVBoxLayout;
        QVBoxLayout *resultLayout = nullptr;
        auto *resultPanel = makePanel("Proposal output", resultLayout);
        auto *result = new QTextEdit;
        result->setReadOnly(true);
        result->setPlainText("No proposal generated yet.\n\nChoose a target and strategy, then select Generate demo proposal.");
        resultLayout->addWidget(result, 1);
        rightCol->addWidget(resultPanel, 1);

        QVBoxLayout *routeOutLayout = nullptr;
        auto *routeOutPanel = makePanel("Route output", routeOutLayout);
        routeOutput_ = new QTextEdit;
        routeOutput_->setReadOnly(true);
        routeOutput_->setPlainText("Select sectors and press Find lowest-cost route.");
        routeOutLayout->addWidget(routeOutput_, 1);
        rightCol->addWidget(routeOutPanel, 1);
        row->addLayout(rightCol, 1);
        outer->addLayout(row, 1);

        connect(generate, &QPushButton::clicked, this, [this, strategy, horizon, result]() {
            const int idx = maneuverTarget_->currentIndex();
            if (idx < 0 || idx >= objects_.size()) return;
            const SpaceObject &o = objects_[idx];
            QString approach = std::isfinite(o.nearestDist)
                ? QString("%1 at %2 units (%3)").arg(o.nearestName).arg(o.nearestDist, 0, 'f', 2).arg(QString::fromLatin1(getRiskLevel(o.nearestDist)))
                : QString("none (single object)");
            result->setPlainText(
                "ASTRA — DEMO MANEUVER PROPOSAL\n\n"
                "Target: " + o.name +
                "\nClosest approach: " + approach +
                "\nStrategy: " + strategy->currentText() +
                "\nHorizon: " + horizon->currentText() +
                "\n\nStatus: illustrative proposal generated\n"
                "Validation: NOT PERFORMED\n"
                "Fuel / delta-v: NOT CALCULATED\n"
                "Secondary collision check: NOT CONNECTED\n\n"
                "This output is a GUI demonstration only. It is not an actionable spacecraft command.");
        });

        connect(findRouteBtn, &QPushButton::clicked, this, [this]() {
            const int s = sectorStart_->currentIndex();
            const int d = sectorEnd_->currentIndex();
            const astra::RouteResult r = astra::runDijkstra(sectors_, s, d);
            if (!r.reachable) {
                routeOutput_->setPlainText(QString("Sector %1 → Sector %2\n\nTarget is unreachable.").arg(s).arg(d));
                return;
            }
            QStringList hops;
            for (int node : r.path) hops << QString("Sector %1").arg(node);
            routeOutput_->setPlainText(
                QString("Recommended safe path\n  %1\n\nTotal effective cost: %2\n\n"
                        "Demo sector data only — not validated for real mission planning.")
                    .arg(hops.join("  →  ")).arg(r.totalCost, 0, 'f', 2));
        });
        return page;
    }

    QWidget *makeAnalyticsPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Analytics", "High-level prototype indicators for the simulated catalogue.", &outer);
        auto *grid = new QGridLayout;
        grid->setSpacing(14);
        grid->addWidget(metricCard("CATALOGUE SIZE", "0", "Objects in local demo list", &anCatalogue_), 0, 0);
        grid->addWidget(metricCard("DEBRIS OBJECTS", "0", "Synthetic objects", &anDebris_), 0, 1);
        grid->addWidget(metricCard("SATELLITES", "0", "Synthetic objects", &anSats_), 0, 2);
        grid->addWidget(metricCard("MEAN RISK SCORE", "0", "Heuristic 0–100 scale", &anMean_), 0, 3);
        outer->addLayout(grid);
        QVBoxLayout *chartLayout = nullptr;
        auto *panel = makePanel("Risk score distribution", chartLayout);
        barsLayout_ = new QVBoxLayout;
        chartLayout->addLayout(barsLayout_);
        chartLayout->addStretch();
        auto *note = new QLabel("These bars visualize heuristic demo scores, not probability of collision.");
        note->setObjectName("Muted");
        chartLayout->addWidget(note);
        outer->addWidget(panel, 1);
        return page;
    }

    void updateAnalytics()
    {
        if (!barsLayout_) return;
        anCatalogue_->setText(QString::number(objects_.size()));
        anDebris_->setText(QString::number(countType("Debris")));
        anSats_->setText(QString::number(countType("Satellite")));
        anMean_->setText(QString::number(meanRisk(), 'f', 1));

        clearLayout(barsLayout_);
        for (const auto &o : objects_) {
            auto *line = new QHBoxLayout;
            auto *name = new QLabel(o.name);
            name->setMinimumWidth(110);
            auto *bar = new QProgressBar;
            bar->setRange(0, 100);
            bar->setValue(static_cast<int>(o.risk));
            bar->setFormat(QString::number(static_cast<int>(o.risk)) + " / 100");
            line->addWidget(name);
            line->addWidget(bar, 1);
            barsLayout_->addLayout(line);
        }
    }

    QWidget *makeLearnPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Learn ASTRA", "A quick guide to the components of the project.", &outer);
        const QStringList titles = {
            "01  Object catalogue",
            "02  Collision module (collision.c)",
            "03  Space network graph (graph.c)",
            "04  Safe routing (routing.cpp)",
            "05  Maneuver planning",
            "06  Machine-learning extension"
        };
        const QStringList descriptions = {
            "Stores simulated satellites and debris with names, demo-plane coordinates and illustrative orbit attributes.",
            "Computes pairwise Euclidean distances and classifies them SAFE / WARNING (≤ 20) / CRITICAL (≤ 10). Scores shown in the GUI are a heuristic derived from the nearest distance.",
            "Adjacency-matrix graph: objects are nodes, links connect objects within range. Provides BFS, DFS and a fewest-hops route finder.",
            "Dijkstra over sectors with edge cost distance × (1 + risk) + fuel, so a longer but safer path can beat a short risky one.",
            "The proposal text is a placeholder. A real planner needs validated orbital dynamics, constraints and secondary-risk checks.",
            "A trained model could estimate risk from prepared data; accuracy and limits must be documented before use."
        };
        for (int i = 0; i < titles.size(); ++i) {
            QVBoxLayout *cardLayout = nullptr;
            auto *card = makePanel(titles[i], cardLayout);
            auto *desc = new QLabel(descriptions[i]);
            desc->setWordWrap(true);
            desc->setObjectName("Muted");
            cardLayout->addWidget(desc);
            outer->addWidget(card);
        }
        outer->addStretch();
        return page;
    }

    int countType(const QString &type) const
    {
        int n = 0;
        for (const auto &o : objects_) if (o.type == type) ++n;
        return n;
    }

    double meanRisk() const
    {
        if (objects_.isEmpty()) return 0.0;
        double total = 0.0;
        for (const auto &o : objects_) total += o.risk;
        return total / objects_.size();
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("ASTRA");
    QApplication::setOrganizationName("ASTRA Project");
    MainWindow window;
    window.show();
    return app.exec();
}
