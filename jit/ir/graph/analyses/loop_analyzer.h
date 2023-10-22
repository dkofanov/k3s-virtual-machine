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
        std::cout << pref << "  Header: " << header_->Id() << "\n";
        std::cout << pref << "  BackEdges:\n";
        for (auto b : back_edges_) {
        std::cout << pref << "    " << b->Id() << "\n";
        }
        std::cout << pref << "  Reducible: " << reducible_ << "\n";
    }
    
    auto Id() const
    {
        return id_;
    }

private:
    Loop(BasicBlock *header, size_t id) : header_(header), id_(id) {}

    void AppendBackEdge(BasicBlock *edge)
    {
        ASSERT(std::find(back_edges_.begin(), back_edges_.end(), edge) == back_edges_.end());
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
    }

    void CollectBackEdges()
    {
        DFSBackEdges(graph_->GetEntryBlock());
    }

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

private:
    void MarkGB(BasicBlock *b) { b->Mark(gb_); }
    void MarkB(BasicBlock *b) { b->Mark(b_); }
    bool IsGB(BasicBlock *b)
    {
        return b->IsMarked(gb_);
    }

    bool IsB(BasicBlock *b) { return b->IsMarked(gb_) || b->IsMarked(b_); }
private:
    Graph *graph_{};
    Marker gb_{};
    Marker b_{};
};

}