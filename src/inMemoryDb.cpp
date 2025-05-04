#include "inMemoryDb.h"

using namespace std;

/* ─────────────────── helpers ─────────────────── */
void InMemoryDB::inc(const string& v) { ++valCount_[v]; }

void InMemoryDB::dec(const string& v) {
    auto it = valCount_.find(v);
    if (it != valCount_.end() && --(it->second) == 0)
        valCount_.erase(it);
}

/* register first-time change inside the **current** transaction */
void InMemoryDB::record(const string& key) {
    if (txnStack_.empty()) return;      // outside txn
    auto& log = txnStack_.back();
    for (auto& c : log)                 // already logged?
        if (c.key == key) return;
    auto it = db_.find(key);
    log.push_back({ key,
                    it == db_.end() ? nullopt
                                    : optional<string>(it->second) });
}

/* ─────────────────── data ops ─────────────────── */
void InMemoryDB::set(const string& key, const string& val) {
    record(key);
    auto it = db_.find(key);
    if (it != db_.end()) {                  // overwrite → fix counts
        if (it->second == val) return;      // no effective change
        dec(it->second);
        it->second = val;
    } else {
        db_[key] = val;
    }
    inc(val);
}

optional<string> InMemoryDB::get(const string& key) const {
    auto it = db_.find(key);
    return it == db_.end() ? nullopt : optional<string>(it->second);
}

void InMemoryDB::del(const string& key) {
    auto it = db_.find(key);
    if (it == db_.end()) return;            // nothing to do
    record(key);
    dec(it->second);
    db_.erase(it);
}

size_t InMemoryDB::count(const string& val) const {
    auto it = valCount_.find(val);
    return it == valCount_.end() ? 0 : it->second;
}

/* ─────── transaction ops ─────── */
void InMemoryDB::begin() { txnStack_.push_back({}); }

TxnStatus InMemoryDB::rollback() {
    if (txnStack_.empty()) return TxnStatus::NoTransaction;

    auto log = std::move(txnStack_.back());
    txnStack_.pop_back();

    for (auto it = log.rbegin(); it != log.rend(); ++it) {
        string current;
        auto curIt = db_.find(it->key);
        if (curIt != db_.end()) current = curIt->second;

        // restore prior state
        if (it->oldVal) {                       // had old value
            if (curIt == db_.end()) db_[it->key] = *it->oldVal;
            else                    curIt->second = *it->oldVal;
        } else {                                // key originally absent
            db_.erase(it->key);
        }

        /* adjust counts */
        if (!current.empty()) dec(current);
        if (it->oldVal)       inc(*it->oldVal);
    }
    return TxnStatus::Ok;
}

TxnStatus InMemoryDB::commit() {
    if (txnStack_.empty()) return TxnStatus::NoTransaction;
    txnStack_.clear();                          // all changes are already in db_
    return TxnStatus::Ok;
}
