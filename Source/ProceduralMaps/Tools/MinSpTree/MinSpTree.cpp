#include "MinSpTree.h"
#include "Math/Vector.h"
#include "Containers/Map.h"

// Kruskal's algorithm Minimum Spanning tree
vector<pair<FVector2D, FVector2D>> MinSpTree::getMinCostPairs()
{
    _size = _costPairs.size(); 
    fillRootMap();
    FVector2D a, b;
    float cost = 0.f;
    vector<pair<FVector2D, FVector2D>> res;
    for (auto p : _costPairs)
    {
        a = p.second.first;
        b = p.second.second;
        cost = p.first;
        // check if roots are creating a cycle
        if (getRoot(a) != getRoot(b))
        {
            _minCost += cost;
            res.push_back({a,b});
            addPair(a, b);
        }
    }
    return res;
}

// add pair
void MinSpTree::addPair(FVector2D a, FVector2D b)
{
    FVector2D aR = getRoot(a);
    FVector2D bR = getRoot(b);
    _rootMap[aR] = _rootMap[bR];
}

void MinSpTree::clear()
{
    _size = 0;
    _minCost = 0;
    _rootMap.Reset();
}

// adding some circular edges
vector<pair<FVector2D, FVector2D>> MinSpTree::getNaturalCostPairs()
{
    _size = _costPairs.size();
    fillRootMap();
    FVector2D a, b;
    float cost = 0.f;
    vector<pair<FVector2D, FVector2D>> res;
    for (auto p : _costPairs)
    {
        a = p.second.first;
        b = p.second.second;
        cost = p.first;
        // check if roots are creating a cycle
        if (getRoot(a) != getRoot(b))
        {
            _minCost += cost;
            res.push_back({ a,b });
            addPair(a, b);
        }
        else
        {
            if (3 == (rand() % 6))
                res.push_back({a,b});
        }
    }
    return res;
}

// finds the root
FVector2D MinSpTree::getRoot(FVector2D val)
{
    while (_rootMap[val] != val)
    {
        _rootMap[val] = _rootMap[_rootMap[val]];
        val = _rootMap[val];
        
    }
    return val;
}

void MinSpTree::fillRootMap()
{
    // fil rootmap
    for (auto p : _costPairs)
    {
        auto pair = p.second;
        _rootMap.Add(pair.first, pair.first);
        _rootMap.Add(pair.second, pair.second);
    }
}



