#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <climits>
#include <ctime>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace std;
namespace py = pybind11;

// Tile structure to represent each tile in the map
struct Tile {

	int cost;
	int totalCost;
	int row, col;

	int count = 0;

	Tile* parent = nullptr;

	Tile* up = nullptr;
	Tile* down = nullptr;
	Tile* left = nullptr;
	Tile* right = nullptr;

	Tile(int cost, int row, int col) : cost(cost), totalCost(INT_MAX), row(row), col(col) {}
};

vector<string> readMap(const string fileName, int& mapLength, int& mapHeight) {
	ifstream inputMap(fileName);

	if (!inputMap.is_open()) {
		cout << "Error: Could not open file map.txt" << endl;
		return {};
	}

	else {

		vector<string> map;

		// Extract the map into strings
		string line;
		while (getline(inputMap, line)) {
			if (mapLength == 0) {
				mapLength = line.length() - 1;
			}
			map.push_back(line);
			mapHeight++;
		}

		// Remove walls if they exist
		if (map[0][0] == 'x') {
			for (int i = 1; i < mapHeight - 1; i++) {
				map[i] = map[i].substr(1, mapLength - 2);
			}
			mapLength -= 2;
			mapHeight -= 2;

			map.erase(map.begin());
			map.pop_back();
		}

		inputMap.close();

		return map;
	}
}

tuple<vector<string>, int, int> readMapPython(const string fileName) {
	int mapLength = 0;
	int mapHeight = 0;
	vector<string> map = readMap(fileName, mapLength, mapHeight);
	if (map.empty()) {
		return make_tuple(vector<string>(), 0, 0);
	}
	return make_tuple(map, mapLength, mapHeight);
}

vector<int> getReferenceMap(vector<string> map) {
	vector<int> referenceMap;

	for (const string& line : map) {
		for (char tile : line) {
			if (tile == '_') {
				referenceMap.push_back(1);
			}
			else if(tile == 'f') {
				referenceMap.push_back(4);
			}
			else if (tile == 'M') {
				referenceMap.push_back(10);
			}
			else {
				referenceMap.push_back(INT_MAX);
			}
		}
	}

	return referenceMap;
}

vector<Tile*> initializeTileMap(const vector<int>& referenceMap, int mapLength) {
	vector<Tile*> tileMap;

	for (int cost : referenceMap) {
		tileMap.push_back(new Tile(cost, tileMap.size() / mapLength, tileMap.size() % mapLength));
	}

	// Set up the tile connections (up, down, left, right)
	for (int i = 0; i < tileMap.size(); i++) {
		if (tileMap[i]->cost == INT_MAX) continue; // Skip water tiles
		if (i >= mapLength) tileMap[i]->up = tileMap[i - mapLength];
		if (i < tileMap.size() - mapLength) tileMap[i]->down = tileMap[i + mapLength];
		if (i % mapLength != 0) tileMap[i]->left = tileMap[i - 1];
		if ((i + 1) % mapLength != 0) tileMap[i]->right = tileMap[i + 1];
	}

	return tileMap;
}

void createLowestCostPaths(Tile* start, Tile* end) {
	
	queue<Tile*> toVisit;
	toVisit.push(start);
	
	while (!toVisit.empty()) {

		Tile* current = toVisit.front();
		toVisit.pop();

		Tile* neighbors[4] = {current->up, current->down, current->left, current->right};

		for (Tile* neighbor : neighbors) {
			if (!neighbor || neighbor->cost == INT_MAX)
				continue;

			int newTotalCost = current->totalCost + neighbor->cost;

			if (newTotalCost < neighbor->totalCost) {
				neighbor->totalCost = newTotalCost;
				neighbor->parent = current;
				toVisit.push(neighbor);
			}
		}
	}
}

void visualizePaths(const vector<Tile*> &tileMap, Tile* start, vector<string> map, int startIndex, int mapLength) {
	Tile* current = start;

	cout << "Cost to reach each tile from start (S) to end (E): " << current->totalCost << endl;

	while (current && current->totalCost != INT_MAX)
	{
		int row = current->row;
		int col = current->col;

		if (row >= 0 && row < map.size() && col >= 0 && col < map[row].size()) // Ensure within map bounds
		{
			if (current == start)
				map[row][col] = 'X'; // Start tile
			else if (current->totalCost == 0)
				map[row][col] = 'O'; // End tile
			else
				map[row][col] = 'o'; // Path tile
		}

		if (current->totalCost == 0)
		{
			// cout << "Found the end tile!" << endl;
			// cout << "Tiles visited: " << tilesVisited << endl;
			break;
		}

		current = current->parent;
	}

	for (const string &row : map)
		cout << row << endl;
}

void printMap(const vector<Tile*> &tileMap, int mapLength) {
	// print the tile map
	for (int i = 0; i <= tileMap.size() - 1; i++) {
		int totalCost = tileMap[i]->totalCost;
		if (totalCost == INT_MAX)
			cout << "-1 ";
		else if (totalCost < 10)
			cout << " " << totalCost << " ";
		else {
			cout << totalCost << " ";
		}

		if ((i + 1) % mapLength == 0)
				cout << endl;
	}
}

vector<pair<int, int>> findPath(const string fileName, int startRow, int startCol, int endRow, int endCol) {

	vector<pair<int, int>> path;

	// Define map dimensions
	int mapLength = 0;
	int mapHeight = 0;

	// read map into vector of strings (each string is a row of the map)
	vector<string> map = readMap(fileName, mapLength, mapHeight);

	// create reference map with costs for each tile
	vector<int> referenceMap = getReferenceMap(map);

	// create tile map with Tile nodes
	vector<Tile*> tileMap = initializeTileMap(referenceMap, mapLength);

	// Create start and end tiles
	int startIndex = startRow * mapLength + startCol;
	int endIndex = endRow * mapLength + endCol;

	Tile* start = tileMap[startIndex];
	Tile* end = tileMap[endIndex];

	// Start timer
	clock_t startTime = clock();

	start->totalCost = 0;			// Set start tile totalCost to 0
	createLowestCostPaths(start, end); // compute totalCost to each tile from end

	// End timer
	clock_t endTime = clock();
	double elapsedTime = double(endTime - startTime) / CLOCKS_PER_SEC;
	cout << "Time taken to compute paths:\n" << elapsedTime << " seconds\n" << elapsedTime * 1000000 << " microseconds\n";

	// End stores the total cost to reach it from start tile
	cout << "Traversal cost: " << end->totalCost << endl;

	// Traverse path from end to start using parent pointers 
	// end -> child -> grandchild -> ... -> start
	Tile* current = end;
	while (current) {
		path.push_back({current->row, current->col});
		if (current == start)
			break; // Reached the end (start) tile
		current = current->parent;
	}

	// Print the map for debugging
	// printMap(tileMap, mapLength);

	// Clean up dynamically allocated memory
	for (Tile* tile : tileMap) {
		delete tile;
	}

	return path;
}

// Pybind module
PYBIND11_MODULE(pathfinder, m) {
	m.doc() = "AI Pathfinding Module";

	m.def("find_path", &findPath,
		  "Find shortest path from (startRow, startCol) to (endRow, endCol)",
		  py::arg("fileName") = "map.txt", py::arg("startRow"), py::arg("startCol"), py::arg("endRow"), py::arg("endCol"));

	m.def("read_map", &readMapPython, 
		  "Read map from file"),
		  py::arg("fileName") = "map.txt";
}