#pragma once

#include "../graph.h"
#include <algorithm>
#include <string>

namespace compiler {

class LoopAnalyzer;

class Loop {
public:
    auto *Header() {
        return header_;
    }

    const auto &BackEdges() {
        return back_edges_;
    }

    auto *Outer()
    {
        return outer_;
    }

    void SetOuter(Loop *outer)
    {
        ASSERT(outer != this);
        ASSERT(!HasInner(outer));
        if (outer_ != nullptr) {
            ASSERT(outer == outer_);
            ASSERT(outer->HasInner(this));
            return;
        }
        outer_ = outer;
        outer->AppendInner(this);
    }

    bool HasInner(Loop *inner) const
    {
        return std::find(inner_.begin(), inner_.end(), inner) != inner_.end();
    }

    auto Reducible()
    {
        return reducible_;
    }

    void SetReducible(bool p)
    {
        reducible_ = p;
    }

    void Dump(size_t rec_lvl = 1)
    {
        auto pref = std::string(rec_lvl * 2, '.');
        std::cout << pref << "Loop(" << id_ << ")\n";
        if (id_ == 0) {
            std::cout << pref << "  Outer: none\n";
        } else {
            std::cout << pref << "  Outer: " << outer_->id_ << "\n";
        }
        std::cout << pref << "  Inner:\n";
        for (auto l : inner_) {
            l->Dump(rec_lvl + 1);
        }
        if (header_ != nullptr) {
        std::cout << pref << "  Header: " << header_->Id() << "\n";
        std::cout << pref << "  BackEdges:\n";
        for (auto b : back_edges_) {
        std::cout << pref << "    " << b->Id() << "\n";
        }
        std::cout << pref << "  Reducible: " << reducible_ << "\n";
        } else {
            std::cout << pref << "  Is root" << "\n";

        }
    }
    
    auto Id() const
    {
        return id_;
    }
    bool IsBackEdge(const BasicBlock *b) const
    {
        return std::find(back_edges_.begin(), back_edges_.end(), b) != back_edges_.end();
    }
private:
    Loop(BasicBlock *header, size_t id) : header_(header), id_(id) {}

    void AppendBackEdge(BasicBlock *edge)
    {
        ASSERT(!IsBackEdge(edge));
        ASSERT(edge->Loop() == nullptr);
        back_edges_.push_back(edge);
        edge->SetLoop(this);
    }

    void AppendInner(Loop *inner)
    {
        ASSERT(!HasInner(inner));
        inner_.push_back(inner);
    }

private:
    BasicBlock *header_{};
    Vector<BasicBlock *> back_edges_{};
    Vector<Loop *> inner_{};
    Loop *outer_{};
    bool reducible_{true};
    size_t id_{};

friend LoopAnalyzer;
friend Graph;
};

class LoopAnalyzer {
public:
    LoopAnalyzer(Graph *graph) : graph_(graph), gb_(graph->NewMarker()),
                                 b_(graph->NewMarker())
    {
        ASSERT(graph_->IsDomTreeValid());

        CollectBackEdges();
        PopulateBlocks();
        CreateRoot();
        PopulateIrreducible();
    }

    void CollectBackEdges()
    {
        DFSBackEdges(graph_->GetEntryBlock());
    }
// TODO: Investigate what does the following comment mean:
/// DEBUG- MARK -ASSERT HAS LOOP 
    void DFSBackEdges(BasicBlock *edge)
    {
        MarkGB(edge);
        for (auto b : edge->Succs()) {
            if (IsGB(b)) {
                ProcessNewBackEdge(edge, b);
            } else if (!IsB(b)) {
                DFSBackEdges(b);
            }
        }
        MarkB(edge);
    }

    void ProcessNewBackEdge(BasicBlock *edge, BasicBlock *b)
    {
        if (b->Loop() == nullptr) {
            b->SetLoop(graph_->NewLoop(b));
        }
        auto l = b->Loop();
        ASSERT(l->Header() == b);
        l->AppendBackEdge(edge);
    }
    
    void PopulateBlocks()
    {
        const auto &rpo = graph_->GetRPO();
        for (auto it = rpo.rbegin(); it != rpo.rend(); it++) {
            auto b = *it;
            auto l = b->Loop();
            if ((l == nullptr) || (l->Header() != b)) {
                continue;
            }
            for (const auto edge : l->BackEdges()) {
                if (graph_->IsDominator(b, edge)) {
                    DFSPopulateBlocks(l, edge);
                } else {
                    l->SetReducible(false);
                    graph_->SetIrreducible();
                }
            }
        }
    }

    void DFSPopulateBlocks(Loop *l, BasicBlock *b)
    {
        if (b->Loop() == nullptr) {
            b->SetLoop(l);
        }
        for (auto pred : b->Preds()) {
            if (pred->Loop() == nullptr) {
                DFSPopulateBlocks(l, pred);
            } else if ((pred->Loop() != l) && b != pred->Loop()->Header()) {
                pred->Loop()->SetOuter(l);
                // Skip inner loop and resume at the header:
                DFSPopulateBlocks(l, pred->Loop()->Header());
            }
        }
    }

    void CreateRoot()
    {
        auto root_loop = graph_->NewLoop();
        for (auto b : graph_->GetRPO()) {
            if (b->Loop() == nullptr) {
                b->SetLoop(root_loop);
            } else if (b->Loop()->Outer() == nullptr) {
                b->Loop()->SetOuter(root_loop);
            }
        }
        graph_->SetRootLoop(root_loop);
    }
    
    void PopulateIrreducible()
    {
        // Reset marker:
        b_ = graph_->NewMarker();
        for (auto b : graph_->GetRPO()) {
            MarkB(b);
            if (!b->IsHeader() || b->Loop()->Reducible()) {
                continue;
            }
            DFSIrreducible(b);
        }
    }

    void DFSIrreducible(BasicBlock *b)
    {
        auto *l = b->Loop();
        ASSERT(!l->Reducible());
        for (auto *pred : b->Preds()) {
            if (IsB(pred)) {
                ASSERT((pred->Loop() != l) || (pred == l->Header()));
                continue;
            }
            ASSERT(!b->IsHeader() ^ l->IsBackEdge(pred));
            pred->SetLoop(l);
            DFSIrreducible(pred);
        }
    }

private:
    void MarkGB(BasicBlock *b) { b->Mark(gb_); }
    void MarkB(BasicBlock *b) { b->Mark(b_); }
    bool IsGB(const BasicBlock *b)
    {
        return b->IsMarked(gb_);
    }

    bool IsB(const BasicBlock *b) { return b->IsMarked(gb_) || b->IsMarked(b_); }
private:
    Graph *graph_{};
    Marker gb_{};
    Marker b_{};
};

}