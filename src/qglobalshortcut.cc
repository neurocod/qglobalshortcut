#include "qglobalshortcut.h"
#include <QCoreApplication>

QMultiHash<quint32, QGlobalShortcut*> QGlobalShortcut::shortcuts_;
QGlobalShortcut::QGlobalShortcutEventFilter QGlobalShortcut::global_shortcut_event_;

QGlobalShortcut::QGlobalShortcut(QObject* parent)
    : QObject(parent)
{
    initialize();
}

QGlobalShortcut::QGlobalShortcut(const QKeySequence &keyseq, QObject *parent)
    : QObject(parent)
{
    initialize();
    setKey(keyseq);
}

void QGlobalShortcut::initialize() {
    static bool initialized = false;
    if (!initialized) {
        qApp->installNativeEventFilter(&global_shortcut_event_);
        initialized = true;
    }
}

QGlobalShortcut::~QGlobalShortcut() {
    unsetKey();
}

QKeySequence QGlobalShortcut::key() const
{
    return keyseq_;
}

bool QGlobalShortcut::setKey(const QKeySequence& keyseq)
{
    if (!keyseq_.isEmpty()) {
        unsetKey();
    }
    bool ret = false;
    quint32 keyid = calcId(keyseq);
    if (shortcuts_.count(keyid) == 0) {
        quint32 keycode = toNativeKeycode(getKey(keyseq));
        quint32 mods = toNativeModifiers(getMods(keyseq));
        ret = registerKey(keycode, mods, keyid);
    }
    this->keyseq_ = keyseq;
    shortcuts_.insert(keyid, this);

    return ret;
}

void QGlobalShortcut::unsetKey() {
    quint32 keyid = calcId(keyseq_);
    if (shortcuts_.remove(keyid, this) > 0) {
        if (shortcuts_.count(keyid) == 0) {
            quint32 keycode = toNativeKeycode(getKey(keyseq_));
            quint32 mods = toNativeModifiers(getMods(keyseq_));
            unregisterKey(keycode, mods, keyid);
        }
    }
}

bool QGlobalShortcut::activate(quint32 id) {
    if (shortcuts_.contains(id)) {
        foreach (QGlobalShortcut* s, shortcuts_.values(id)) {
            emit s->activated();
        }
        return true;
    }
    return false;
}

quint32 QGlobalShortcut::calcId(const QKeySequence& keyseq) {
    quint32 keycode = toNativeKeycode(getKey(keyseq));
    quint32 mods    = toNativeModifiers(getMods(keyseq));
    return calcId(keycode, mods);
}

#ifndef Q_OS_UNIX
quint32 QGlobalShortcut::calcId(quint32 k, quint32 m) {
    return k | m;
}
#endif

Qt::Key QGlobalShortcut::getKey(const QKeySequence& keyseq) {
    if (keyseq.isEmpty())
        return Qt::Key(0);

    QKeyCombination comb = keyseq[0];
    return comb.key();
}

Qt::KeyboardModifiers QGlobalShortcut::getMods(const QKeySequence& keyseq) {
    if (keyseq.isEmpty()) {
        return Qt::KeyboardModifiers(0);
    }
    return keyseq[0].keyboardModifiers();
}
