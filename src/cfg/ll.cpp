#include "include/cfg/ll.h"

#include <list>

namespace atom::cfg::ll {

BadLL::BadLL(std::string_view msg):
    m_msg(msg) {}

const char* BadLL::what() const noexcept {
    return m_msg.data();
}

FirstAndFollowReqHandler::FirstAndFollowReqHandler(ProductionRulesType& prodRules):
m_prodRules(prodRules),
m_firstCacheTable(),
m_followCacheTable()
{}

const FirstAndFollowReqHandler::SymbolSetType& FirstAndFollowReqHandler::requestFollow(const SymbolType& symbol) {
    static const SymbolSetType emptyFollow;
    if (symbol.isTerminal()) {
        return emptyFollow;
    }

    auto it = m_followCacheTable.find(symbol);
    if (it != m_followCacheTable.cend()) {
        return it->second;
    }

    auto follow = makeFollow(symbol);
    if (follow.empty()) {
        return emptyFollow;
    }

    it = m_followCacheTable.insert(std::make_pair(symbol, follow)).first;
    assert(it != m_followCacheTable.cend());
    return it->second;
}

const FirstAndFollowReqHandler::SymbolSetType& FirstAndFollowReqHandler::requestFirst(const SymbolType& symbol) {
    static const SymbolSetType emptyFirst;
    auto it = m_firstCacheTable.find(symbol);
    if (it != m_firstCacheTable.cend()) {
        return it->second;
    }

    auto first = makeFirst(symbol);
    if (first.empty()) {
        return emptyFirst;
    }

    it = m_firstCacheTable.insert(std::make_pair(symbol, first)).first;
    assert(it != m_firstCacheTable.cend());
    return it->second;
}

FirstAndFollowReqHandler::SymbolSetType FirstAndFollowReqHandler::makeFirst(const SymbolType& symbol) {
    using namespace atom::cfg::grammar;
    SymbolSetType first;
    std::list<SymbolType> context;
    context.push_back(symbol);

    while (!context.empty()) {
        const auto searchSymbol = context.back();
        context.pop_back();

        if (searchSymbol.isTerminal()) {
            first.insert(Terminal{ searchSymbol.getData() });
            continue;
        }

        auto it = m_prodRules.find(NonTerminal{ searchSymbol.getData() });
        if (it == m_prodRules.end()) {
            assert(false);
            return first;
        }

        const auto& [_, prods] = *it;
        for (auto prodIt = prods.cbegin(); prodIt != prods.cend(); ++prodIt) {
            SymbolSetType tmpFirst;
            const ProductionType& prod = *prodIt;
            assert(!prod.isEmpty());

            const DerivationType& firstDerivation = *prod.cbegin();
            if (firstDerivation.isTerminal()) {
                tmpFirst.insert(Terminal{ firstDerivation.getData() });
            } else {
                auto cacheIt = m_firstCacheTable.find(firstDerivation);
                if (cacheIt != m_firstCacheTable.cend()) {
                    tmpFirst.insert(cacheIt->second.begin(), cacheIt->second.end());
                } else {
                    context.push_back(static_cast<SymbolType>(firstDerivation));
                }
            }

            if (auto _it = tmpFirst.find(None);
                _it != tmpFirst.end()) {
                tmpFirst.erase(_it);

                const auto nextSymbol = m_prodRules.findNextDerivation(searchSymbol);
                if (nextSymbol.has_value()) {
                    if (nextSymbol->isTerminal()) {
                        tmpFirst.insert(*nextSymbol);
                    } else {
                        auto cacheIt = m_firstCacheTable.find(*nextSymbol);
                        if (cacheIt != m_firstCacheTable.cend()) {
                            tmpFirst.insert(cacheIt->second.begin(), cacheIt->second.end());
                        } else {
                            context.push_back(static_cast<SymbolType>(*nextSymbol));
                        }
                    }
                } else {
                    tmpFirst.insert(None);
                }

                first.insert(tmpFirst.cbegin(), tmpFirst.cend());
            }

            first.insert(tmpFirst.cbegin(), tmpFirst.cend());
        }
    }
    return first;
}

FirstAndFollowReqHandler::SymbolSetType FirstAndFollowReqHandler::makeFollow(const SymbolType& symbol) {
    using namespace atom::cfg::grammar;
    SymbolSetType follow;
    const auto nextSymbol = m_prodRules.findNextDerivation(symbol);
    if (!nextSymbol.has_value()) {
        follow.insert(End);
        return follow;
    }

    const auto& first = requestFirst(static_cast<SymbolType>(*nextSymbol));
    if (first.empty()) {
        follow.insert(End);
        return follow;
    }

    follow.insert(first.cbegin(), first.cend());
    if (auto it = follow.find(None); it != follow.cend()) {
        follow.erase(it);
        follow.insert(End);
    }
    return follow;
}

bool operator==(const FirstAndFollowReqHandler::SymbolSetType& l, const FirstAndFollowReqHandler::SymbolSetType& r) {
    if (l.size() != r.size()) {
        return false;
    }

    for (const auto& item : l) {
        if (r.find(item) == r.cend()) {
            return false;
        }
    }

    return true;
}

} //! namespace atom::cfg::ll
