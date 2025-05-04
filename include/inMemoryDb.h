#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

/*
 * Simple in-memory, single-threaded database with nested transactions.
 * All data operations run in expected O(1) (hash-map) time, which is
 * within the O(log N) worst-case bound that was requested.
 *
 * ────────────────────────────────────────────────────────────────
 *  Data structures
 * ────────────────────────────────────────────────────────────────
 *
 *  db_          : current committed key → value map
 *  valCount_    : current committed value → multiplicity (COUNT) map
 *
 *  txnStack_    : vector< vector<Change> >
 *                 each level holds the “undo log” of THAT transaction
 *                 (only the first modification of a key in the scope
 *                 is stored, so space is proportional to #changes,
 *                 not #keys → satisfies space constraint).
 *
 *  Change       : { string key; optional<string> priorValue; }
 *                 priorValue == nullopt  ⇒  key was absent beforehand
 *
 *  On ROLLBACK  : replay undo log in reverse and pop one level
 *  On COMMIT    : discard the entire stack (logs already applied)
 */

/* Return‑code for COMMIT / ROLLBACK */
enum class TxnStatus { Ok, NoTransaction };

class InMemoryDB {
    /* per‑transaction undo record */
    struct Change {
        std::string                 key;
        std::optional<std::string>  oldVal;      // nullopt ⇒ key was absent
    };

    std::unordered_map<std::string,std::string> db_;        // key → value
    std::unordered_map<std::string,std::size_t> valCount_;  // value → freq

    std::vector<std::vector<Change>>            txnStack_;  // stack of undo logs (one per BEGIN)

    void inc(const std::string& v);             // ++count[v]
    void dec(const std::string& v);             // --count[v]
    void record(const std::string& key);        // log first change in txn

public:
    /* data commands */
    void                       set  (const std::string& key,
                                     const std::string& val);
    std::optional<std::string> get  (const std::string& key) const;
    void                       del  (const std::string& key);
    std::size_t                count(const std::string& val) const;

    /* transaction commands */
    void       begin();
    TxnStatus  rollback();      // Ok | NoTransaction (if stack empty)
    TxnStatus  commit();        // Ok | NoTransaction (same behaviour)
};