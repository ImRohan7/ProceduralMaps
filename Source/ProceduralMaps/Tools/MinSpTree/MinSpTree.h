#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

struct FVector2D;

class MinSpTree {


public:
    vector<pair<FVector2D, FVector2D>> getMinCostPairs();
    inline float getCost() { return _minCost; };
    FVector2D getRoot(FVector2D val);
    void fillRootMap();
    void addPair(FVector2D a, FVector2D b);
    void clear();

    // custom for real dungeon graph and adding more pairs
    vector<pair<FVector2D, FVector2D>> getNaturalCostPairs();

public:
    vector<pair<float, pair<FVector2D, FVector2D>>> _costPairs;
private:
    TMap<FVector2D, FVector2D> _rootMap;
    float _minCost;
    int _size;
};