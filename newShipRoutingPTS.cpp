/*Head Files*/
#include<bits/stdc++.h>/*Universal Head File*/
#include<random>

using namespace std;

/*---------------------------------objectives & file operations---------------------------------*/
/*Global variables*/
string DataFileName;/*container for data file name*/
int J;/*the number of the types of boats*/
int H;/*the number of routes*/
bool fea = 0;/*feasibility*/

/*Variables processed uniformly*/
double Ljh = 0.6;/*Average shipping space utilization rate*/
int Tsjh = 2;/*Berthing time per round trip voyage*/
int CRjh = 3960000;/*Annual operatng cost*/

/*global parameters*/
int Pr = 182;/*price of heavy oil*/
int Pc = 341;/*price of light oil*/

/*class for boats*/
class boats {
public:
	int Bj;/*the boat's number*/
	//bool Sj;/*the boat's usage state, 1 for yes, 0 for no*/
	int Uj;/*the boat's rated carrying-capacity*/
	int Fj;/*the boat's yearly idle costs*/
	int Tj;/*the boat's yearly operation days*/
	int Mj;/*the boat's yearly rent costs*/
	double Vj;/*the boat's speed*/
	double minVj;/*the speed lower bound*/
	double maxVj;/*the speed upper bound*/
	double OHj, OEj, OLj;/*oil consume cofficients*/

	void speedInit() {
		this->Vj = this->maxVj;
	}
private:
}*B, * bestB;/*initial boats and best solution boats*/

/*class for routes*/
class routes {
public:
	int Rh;/*the route's number*/
	int Wh;/*the route's yearly traffic volume*/
	int residualWh;/*the route's residual traffic volume*/
	double Dh;/*distance*/
	int tEh;/*the route's dispatch interval*/
	int Ph;/*the route's total port fee*/
	int Sh = 0; /*the route's demand state, 1 for satisfied, 0 for not satisfied, 2 for oversatisfied*/
	bool Nh = 0; /*the route's Nh State, 1 for satisfied, 0 for not satisfied*/

	void residualInit() {
		this->residualWh = this->Wh;
	}
private:
} * R, *bestR; /*initial routes*/

/*function to count item number*/
int CountItemNumbers(string DataFileName) {
	ifstream ReadFile;
	int DataNumber = 0;
	string tmp;
	ReadFile.open(DataFileName, ios::in);//ios::in aka only read the data
	if (ReadFile.fail())//fail to open the file: return 0
	{
		cout << "Sorry, fail to open the file, please make sure the file exists.";
		return 0;
	}
	else//file exists
	{
		while (getline(ReadFile, tmp, '\n'))
		{
			DataNumber++;
		}
		ReadFile.close();
		return DataNumber;
	}
}

/*input boat's info*/
void io_boats() {
	ifstream file;
	int LINES;
	file.open("boats.txt", ios::in);
	if (file.fail()) {
		cout << "Failed to open file." << endl;
		file.close();
	}
	else {
		LINES = CountItemNumbers("boats.txt");
		cout << "Data File Reading Finished! There are " << LINES << " boats." << endl;
		B = new boats[LINES];
		int j = 0;
		while (!file.eof()) {
			file >> B[j].Bj;
			file >> B[j].Uj;
			file >> B[j].Fj;
			file >> B[j].Tj;
			file >> B[j].Mj;
			file >> B[j].minVj;
			file >> B[j].maxVj;
			file >> B[j].OHj;
			file >> B[j].OEj;
			file >> B[j].OLj;
			//file >> B[j].Sj;
			j++;
		}
		file.close();
		for (j = 0; j < LINES; j++) {
			cout << "No." << B[j].Bj << " boat's info:" << endl;
			cout << "The boat's rated carrying-capacity: " << B[j].Uj << endl;
			cout << "The boat's yearly idle costs: " << B[j].Fj << endl;
			cout << "The boat's yearly operation days: " << B[j].Tj << endl;
			cout << "The boat's yearly rent costs: " << B[j].Mj << endl;
			cout << "The boat's min speed: " << B[j].minVj << endl;
			cout << "The boat's max speed: " << B[j].maxVj << endl;
			cout << "The boat's oil consuming cofficients: " << B[j].OHj << "\t" << B[j].OEj << "\t" << B[j].OLj << endl;
			//cout << "The boat's usage state: " << B[j].Sj << endl;
		}
		//delete[]B;
	}
}

/*input route's info*/
void io_routes() {
	ifstream file;
	int LINES;
	file.open("routes.txt", ios::in);
	if (file.fail()) {
		cout << "Failed to open file." << endl;
		file.close();
	}
	else {
		LINES = CountItemNumbers("routes.txt");
		cout << "Data File Reading Finished! There are " << LINES << " routes." << endl;
		R = new routes[LINES];
		int h = 0;
		while (!file.eof()) {
			file >> R[h].Rh;
			file >> R[h].Wh;
			file >> R[h].Dh;
			file >> R[h].tEh;
			file >> R[h].Ph;
			//file >> R[h].Sh;
			//file >> R[h].Nh;
			h++;
		}
		file.close();
		for (h = 0; h < LINES; h++) {
			R[h].residualInit();
			cout << "No." << R[h].Rh << " route's info:" << endl;
			cout << "The route's yearly traffic volume: " << R[h].Wh << endl;
			cout << "The route's residual yearly traffic volume: " << R[h].residualWh << endl;
			cout << "The route's distance: " << R[h].Dh << endl;
			cout << "The route's dispatch interval: " << R[h].tEh << endl;
			cout << "The route's total port fee: " << R[h].Ph << endl;
			cout << "The route's demand state: " << R[h].Sh << endl;
			cout << "The route's number state: " << R[h].Nh << endl;
		}
		//delete[]R;
	}
}

/*---------------------------------function tools---------------------------------*/
/*random number generator*/
int randNum(int s) {
	unsigned seed = time(NULL) * 1000;
	std::uniform_int_distribution<> distr(0, s); //uniform_real_distribution, gengerate real number
	std::mt19937 gen(seed); //"std::default_random_engine" is also available??mt19937 better
	auto randInt = std::bind(distr, gen);

	return randInt();
}

/*add boats to idle route at first*/
void initRouteLists(vector<vector<int> >& routeList) {
	for (int j = 0; j < J; j++) {
		routeList[H].push_back(B[j].Bj);
		//cout << routeList[H][j] << "\t";
	}
}

/*find max redisual freight demand in all routes*/
int findMaxRoute() {
	int maxRWh = R[0].residualWh;
	int maxRWhIndex = R[0].Rh;
	cout << "The residual freight denmand of route " << R[0].Rh << " is: " << R[0].residualWh << endl;
	for (int h = 1; h < H; h++) {
		cout << "The residual freight denmand of route " << R[h].Rh << " is: " << R[h].residualWh << endl;
		if (R[h].residualWh > maxRWh) {
			maxRWh = R[h].residualWh;
			maxRWhIndex = R[h].Rh;
		}
	}
	/*FIXME: If all routes' demand are met,the function will return -1*/
	cout << "Now route " << maxRWhIndex << " has the max residual freight denmand: " << maxRWh << "." << endl;
	//cout << "maxResidualWh = " << maxRWh << endl;
	//cout << "maxResidualWhIndex = " << maxRWhIndex << endl;
	return maxRWhIndex;
}

/*find max boat in certain route*/
int findMaxBoat(int maxRWhIndex, int** Q) {
	cout << endl;
	cout << "Finding max freight boat on route " << maxRWhIndex << "......" << endl;
	//outputQ(Q);
	int maxBIndex = 1;
	int max = Q[0][maxRWhIndex - 1];
	//cout << max << endl;
	for (int j = 1; j < J; j++) {
		if (Q[j][maxRWhIndex - 1] > max) {
			max = Q[j][maxRWhIndex - 1];
			maxBIndex = B[j].Bj;
		}
	}
	cout << "The max boat on route " << maxRWhIndex << " is boat " << maxBIndex << "." << endl;
	cout << "The maximum traffic volume is " << max << endl;
	cout << endl;
	return maxBIndex;
}

/*find min boat in idle route*/
int findMinIdleBoat(int Rindex, vector<vector<int> >& routeList) {
	int minBIndex;
	int min;
	int temp;
	cout << endl;

	if (routeList[H].size() == 0) {
		cout << "There is no boat on idle route." << endl;
		minBIndex = -1;
	}
	else {
		cout << "Boats in idle route are: " << endl;
		for (int j = 0; j < routeList[H].size(); j++) {
			cout << routeList[H][j] << "\t";
		}
		cout << endl;
		minBIndex = routeList[H][0];
		min = floor(B[routeList[H][0] - 1].Tj / (R[Rindex - 1].Dh / (24 * B[routeList[H][0] - 1].Vj) + Tsjh)) * floor(Ljh * B[routeList[H][0] - 1].Uj);

		for (int j = 1; j < routeList[H].size(); j++) {
			temp = floor(B[routeList[H][j] - 1].Tj / (R[Rindex - 1].Dh / (24 * B[routeList[H][j] - 1].Vj) + Tsjh)) * floor(Ljh * B[routeList[H][j] - 1].Uj);
			if (temp < min && temp != 0) {
				min = temp;
				minBIndex = routeList[H][j];
			}
		}
		cout << "The min boat on idle route for route " << Rindex << " is boat " << minBIndex << "." << endl;
		cout << "The minimum traffic volume is " << min << endl;
		cout << endl;
	}
	return minBIndex;
}

/*find min boat in certain route*/
int findMinUsedBoat(int Rindex, vector<vector<int> >& routeList) {
	int minBIndex;
	int min;
	int temp;

	if (routeList[Rindex - 1].size() == 0) {
		cout << "There is no boat on route " << Rindex << "." << endl;
		exit(-1);
	}
	else {
		cout << "Boats in route " << Rindex << " are: " << endl;
		for (int j = 0; j < routeList[Rindex - 1].size(); j++) {
			cout << routeList[Rindex - 1][j] << "\t";
		}
		cout << endl;

		minBIndex = routeList[Rindex - 1][0];
		min = floor(B[routeList[Rindex - 1][0] - 1].Tj / (R[Rindex - 1].Dh / (24 * B[routeList[Rindex - 1][0] - 1].Vj) + Tsjh)) * floor(Ljh * B[routeList[Rindex - 1][0] - 1].Uj);

		for (int j = 1; j < routeList[Rindex - 1].size(); j++) {
			temp = floor(B[routeList[Rindex - 1][j] - 1].Tj / (R[Rindex - 1].Dh / (24 * B[routeList[Rindex - 1][j] - 1].Vj) + Tsjh)) * floor(Ljh * B[routeList[Rindex - 1][j] - 1].Uj);
			if (temp < min && temp != 0) {
				min = temp;
				minBIndex = B[routeList[Rindex - 1][j] - 1].Bj;
			}
		}
		cout << "The min boat on route " << Rindex << " is boat " << minBIndex << "." << endl;
		cout << "The minimum traffic volume is " << min << endl;
		cout << endl;
		return minBIndex;
	}
}

/*find a boat's route*/
int findRoute(vector<vector<int> >& routeList, int Bindex) {
	vector<int> ::iterator iter;
	int Rindex = 0;
	for (int h = 0; h < routeList.size(); h++) {
		iter = find(routeList[h].begin(), routeList[h].end(), Bindex);
		if (iter != routeList[h].end()) {
			Rindex = h + 1;
			//cout << index << endl;
			break;
		}
	}
	cout << "Boat " << Bindex << " is on route " << Rindex << "." << endl;
	return Rindex;
}


/*add min boat of route 1 to route 2*/
void add(int Rindex1, int Rindex2, vector<vector<int> >& routeList) {
	int Bindex;
	vector<int> ::iterator iter;
	int dis;
	Bindex = findMinUsedBoat(Rindex1, routeList);
	routeList[Rindex2 - 1].push_back(Bindex);
	iter = find(routeList[Rindex1 - 1].begin(), routeList[Rindex1 - 1].end(), Bindex);
	dis = std::distance(routeList[Rindex1 - 1].begin(), iter);
	routeList[Rindex1 - 1].erase(routeList[Rindex1 - 1].begin() + dis);
	cout << "Boat " << Bindex << " is inserted to route " << Rindex2 << "." << endl;
}

/*initial tabu list for swap*/
void initTL(int** TL, int tl) {
	for (int i = 0; i < J; i++) {
		for (int j = 0; j < J; j++) {
			if (i == j) {
				TL[i][j] = 0;
			}
			else {
				TL[i][j] = tl;
			}
			//cout << TL[i][j] << " ";
		}
		//cout << endl;
	}
}

/*output route lists*/
void outputRoutes(vector<vector<int> >& routeList) {
	for (int h = 0; h < routeList.size() - 1; h++) {
		cout << "Boats in route " << R[h].Rh << " are: " << endl;
		for (int j = 0; j < routeList[h].size(); j++) {
			cout << routeList[h][j] << "\t";
		}
		cout << endl;
	}

	cout << "Boats in idle route are: " << endl;
	for (int j = 0; j < routeList[H].size(); j++) {
		cout << routeList[H][j] << "\t";
	}
	cout << endl;
}

/*output boats' speed*/
void outputBoatspeed(vector<vector<int> >& routeList) {
	for (int h = 0; h < routeList.size() - 1; h++) {
		cout << "Speed of boats in route " << R[h].Rh << " are: " << endl;
		for (int j = 0; j < routeList[h].size(); j++) {
			cout << B[routeList[h][j] - 1].Vj << "\t";
		}
		cout << endl;
	}

	cout << "Speed of boats in idle route are: " << endl;
	for (int j = 0; j < routeList[H].size(); j++) {
		cout << B[routeList[H][j] - 1].Vj << "\t";
	}
	cout << endl;
}

/*update flights table N*/
void updateN(int** N) {
	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H; h++) {
			N[j][h] = floor(B[j].Tj / (R[h].Dh / (24 * B[j].Vj) + Tsjh));
		}
	}
}

/*update freight volume table Q*/
void updateQ(int** Q, int** N) {
	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H; h++) {
			Q[j][h] = floor(Ljh * B[j].Uj) * N[j][h];
		}
	}
}

/*output table N*/
void outputN(int** N) {
	cout << "Numbers of flights of boat j in route h: " << endl;
	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H; h++) {
			cout << N[j][h] << "\t";
		}
		cout << endl;
	}
}

/*output table Q*/
void outputQ(int** Q) {
	cout << "Freight volume of boat j in route h: " << endl;
	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H; h++) {
			cout << Q[j][h] << "\t";
		}
		cout << endl;
	}
}

/*update routes' residual freight demand*/
void updateResidualWh(vector<vector<int> >& routeList, int** Q) {
	int j, h;
	cout << endl;
	for (h = 0; h < routeList.size() - 1; h++) {
		for (j = 0; j < routeList[h].size(); j++) {
			//cout << routeList[h][j] << "\t";
			R[h].residualWh -= Q[routeList[h][j] - 1][h];
		}
	}
}

/*output routes' residual freight demand*/
void outputResidualWh(vector<vector<int> >& routeList, int** Q) {
	int j, h;
	for (h = 0; h < routeList.size() - 1; h++) {
		cout << "The residual freight denmand of route " << R[h].Rh << " is: " << R[h].residualWh << endl;
	}
}

/*update oil costs*/
double updateCO(vector<vector<int> >& routeList, int** N) {
	double* re = new double[J];
	int i = 0;
	double sum = 0;
	for (int h = 0; h < routeList.size() - 1; h++) {
		for (int j = 0; j < routeList[h].size(); j++) {
			//cout << N[routeList[h][j] - 1][h] << "* (" << Pr << " *" << B[routeList[h][j] - 1].OHj << "*" << B[routeList[h][j] - 1].Vj << "*" << B[routeList[h][j] - 1].Vj << "*" << B[routeList[h][j] - 1].Vj << "* (" << R[h].Dh << "/ (" << 24 << "*" << B[routeList[h][j] - 1].Vj << "))+" << Pc << "* (" << B[routeList[h][j] - 1].OEj << "* (" << R[h].Dh << "/ (" << 24 << "*" << B[routeList[h][j] - 1].Vj << ")) +" << B[routeList[h][j] - 1].OLj << "*" << Tsjh << "))" << endl;
			re[i] = N[routeList[h][j] - 1][h] * (Pr * B[routeList[h][j] - 1].OHj * B[routeList[h][j] - 1].Vj * B[routeList[h][j] - 1].Vj * B[routeList[h][j] - 1].Vj * (R[h].Dh / (24 * B[routeList[h][j] - 1].Vj))
				+ Pc * (B[routeList[h][j] - 1].OEj * (R[h].Dh / (24 * B[routeList[h][j] - 1].Vj)) + B[routeList[h][j] - 1].OLj * Tsjh));
			i += 1;
		}
	}
	for (int t = 0; t < i; t++) {
		//cout << re[t] << endl;
		sum += re[t];
	}

	return sum;
}

/*update port costs*/
double updateCP(vector<vector<int> >& routeList, int** N) {
	double* rf = new double[J];
	int i = 0;
	double sum = 0;
	for (int h = 0; h < routeList.size() - 1; h++) {
		for (int j = 0; j < routeList[h].size(); j++) {
			rf[i] = N[routeList[h][j] - 1][h] * R[h].Ph;
			i++;
		}
	}
	for (int t = 0; t < i; t++) {
		sum += rf[t];
	}

	return sum;
}

/*output the subject*/
double calSubject(vector<vector<int> >& routeList, int** N) {
	int i = 0;
	double CO = updateCO(routeList, N);/*oil cost*/
	double CP = updateCP(routeList, N);/*port cost*/
	double sum = 0;
	sum = CO + CP;
	double M = 0, CR = 0, idle = 0;/*rental cost, operation cost, idle costs*/
	for (int h = 0; h < routeList.size() - 1; h++) {
		for (int j = 0; j < routeList[h].size(); j++) {
			M += B[routeList[h][j] - 1].Mj;
			CR += CRjh;
			/*TODO: add variable CRjh(operation cost) and Tsjh(days)*/
		}
	}
	for (int j = 0; j < routeList[H].size(); j++) {
		idle += B[routeList[H][j] - 1].Fj;
	}
	sum += CR + M + idle;

	cout << endl;
	cout << "Base on current solution:" << endl;
	cout << "Operation costs: " << CR << endl;
	cout << "Oil costs: " << CO << endl;
	cout << "Port costs: " << CP << endl;
	cout << "Rent costs: " << M << endl;
	cout << "Idle costs:" << idle << endl;
	cout << "Total costs: " << sum << endl;

	return sum;
}


/*update minimum number of ships for every route*/
void updateNh(double* Nh) {
	double** nh = new double* [H];/*create rows*/
	for (int h = 0; h < H; h++) {
		nh[h] = new double[J];/*create columns*/
	}

	for (int h = 0; h < H; h++) {
		for (int j = 0; j < J; j++) {
			nh[h][j] = R[h].Dh / (24 * B[j].Vj) + Tsjh;
		}
	}

	for (int h = 0; h < H; h++) {
		Nh[h] = 0;
		for (int j = 0; j < J; j++) {
			Nh[h] += nh[h][j];
			//cout << nh[h][j] << "\t";
		}
		Nh[h] /= J;
		Nh[h] /= R[h].tEh;
		Nh[h] = ceil(Nh[h]);
		cout << "Route " << R[h].Rh << "'s least ship number is: " << Nh[h] << endl;
	}
	cout << endl;
}

/*Nh State(all routes)*/
bool NhState(double* Nh, vector<vector<int> >& routeList) {
	bool s = 1;
	for (int h = 0; h < routeList.size() - 1; h++) {
		if (routeList[h].size() < Nh[h]) {
			s = 0;
			//cout << "Route " << R[h].Rh << " needs more boats for operation." << endl;
		}
	}
	return s;
}

/*TODO:a function to update NhState of each route*/
void updateRouteNhState(double* Nh, vector<vector<int> >& routeList){
	for (int h = 0; h < routeList.size() - 1; h++) {
		if (routeList[h].size() < Nh[h]) {
			R[h].Nh = 0;
			cout << "Route " << R[h].Rh << "'s Nh state: " << R[h].Nh << endl;
		}
		else if (routeList[h].size() == Nh[h]) {
			R[h].Nh = 1;
			cout << "Route " << R[h].Rh << "'s Nh state: " << R[h].Nh << endl;
		}
		else {
			R[h].Nh = 2;
			cout << "Route " << R[h].Rh << "'s Nh state: " << R[h].Nh << endl;
		}
	}
}

/*adjust ship number of route Rindex*/
void NhAdjust(double* Nh, vector<vector<int> >& routeList, int Rindex) {
	int Bindex, dis, r;
	vector<int> ::iterator iter;
	
	while (routeList[Rindex - 1].size() < Nh[Rindex - 1]) {
		cout << "Route " << Rindex << " needs more ships to operate.Nh adjusting......" << endl;
			Bindex = findMinIdleBoat(Rindex, routeList);
			cout << "Bindex = " << Bindex << endl;
			if (Bindex == -1) {
				cout << "No boats on idel route." << endl;
				cout << "Try to find boat on another route." << endl;
			}
			else {
				cout << "Assign boat " << Bindex << " to route " << Rindex << "." << endl;
				routeList[Rindex - 1].push_back(Bindex);
				iter = find(routeList[H].begin(), routeList[H].end(), Bindex);
				if (iter != routeList[H].end()) {
					dis = std::distance(routeList[H].begin(), iter);
					//cout << index << endl;
				}
				else
				{
					cout << "ERROR! Can't find the boat." << endl;
					exit(-1);
				}
				routeList[H].erase(routeList[H].begin() + dis);
			}
		}
}

void updateRouteDemandState(vector<vector<int> >& routeList, int** Q, int** N) {
	int Bindex;
	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	updateN(N);
	updateQ(Q, N);
	updateResidualWh(routeList, Q);
	//outputResidualWh(routeList, Q);

	for (int h = 0; h < routeList.size() - 1; h++) {
		Bindex = findMinUsedBoat(h + 1, routeList);
		if (R[h].residualWh > 0) {
			R[h].Sh = 0;
			//cout << "Route " << R[h].Rh << " does not meet its demand." << endl;
			cout << "Route " << R[h].Rh << "'s demand state: " << R[h].Sh << endl;
		}
		else if (R[h].residualWh + Q[Bindex - 1][h] <= 0) {
			R[h].Sh = 2;
			//cout << "Route " << R[h].Rh << " has extra ships." << endl;
			cout << "Route " << R[h].Rh << "'s demand state: " << R[h].Sh << endl;
		}
		else {
			R[h].Sh = 1;
			//cout << "Route " << R[h].Rh << " is good." << endl;
			cout << "Route " << R[h].Rh << "'s demand state: " << R[h].Sh << endl;
		}
	}
}

/*create initial assignment*/
void initAssignment(vector<vector<int> >& routeList, int** Q, int** N) {
	int j, h, Bindex, Rindex;
	int index;

	int** q = new int* [J];/*create rows*/
	for (int j = 0; j < J; j++) {
		q[j] = new int[H];/*create columns*/
	}

	vector<int> ::iterator iter;

	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	for (int j = 0; j < J; j++) {
		B[j].speedInit();
	}
	updateN(N);
	updateQ(Q, N);
	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H; h++) {
			q[j][h] = Q[j][h];
		}
	}
	//outputQ(q);
	//outputRoutes(routeList);
	Rindex = findMaxRoute();
	//cout << Rindex << endl;
	Bindex = findMaxBoat(Rindex, Q);
	//cout << Bindex << endl;
	while (Rindex >= 0 && R[Rindex - 1].residualWh > 0) {
		routeList[Rindex - 1].push_back(Bindex);
		iter = find(routeList[H].begin(), routeList[H].end(), Bindex);
		if (iter == routeList[H].end()) {
			cout << "ERROR! Can't find the boat." << endl;
			exit(-1);
		}
		else
		{
			index = std::distance(routeList[H].begin(), iter);
			//cout << index << endl;
		}
		routeList[H].erase(routeList[H].begin() + index);

		for (int h = 0; h < routeList.size() - 1; h++) {
			R[h].residualInit();
		}
		updateResidualWh(routeList, Q);
		//outputResidualWh(routeList, Q);

		for (int h = 0; h < H; h++) {
			q[Bindex - 1][h] = 0;
		}
		/*cout << "Update Q after assigning boat " << Bindex << " to route " << Rindex << ":" << endl;
		outputQ(q);*/

		Rindex = findMaxRoute();
		if (Rindex >= 0 && R[Rindex - 1].residualWh > 0) {
			Bindex = findMaxBoat(Rindex, q);
		}
	}
}

/*---------------------------------Probabilistic Tabu Search Functions---------------------------------*/
/*swap boats, swap boat 1 on route 1 to boat 2 on route 2*/
void swap(vector<vector<int> >& routeList, int Bindex1, int Bindex2, int Rindex1, int Rindex2) {
	int dis1, dis2;
	vector<int> ::iterator iter;
	cout << "Swap boats: " << Bindex1 << "\t" << Bindex2 << endl;

	routeList[Rindex1 - 1].push_back(Bindex2);
	iter = find(routeList[Rindex1 - 1].begin(), routeList[Rindex1 - 1].end(), Bindex1);
	dis1 = std::distance(routeList[Rindex1 - 1].begin(), iter);
	routeList[Rindex1 - 1].erase(routeList[Rindex1 - 1].begin() + dis1);
	routeList[Rindex2 - 1].push_back(Bindex1);
	iter = find(routeList[Rindex2 - 1].begin(), routeList[Rindex2 - 1].end(), Bindex2);
	dis2 = std::distance(routeList[Rindex2 - 1].begin(), iter);
	routeList[Rindex2 - 1].erase(routeList[Rindex2 - 1].begin() + dis2);	
}

/*Probabilistic tabu swap*/
void PTswap(vector<vector<int> >& routeList, int** TL, int tl, double bestSoFar, int** N, int**Q, double* Nh) {
	int s = 0;
	int r;/*random number*/
	int Bindex1, Bindex2;
	int Rindex1 = 0, Rindex2 = 0;
	int index;
	double currentSubject = 0;

	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Swap boats(PTS)......" << endl;
	do {
		cout << "Choosing boats to swap......" << endl;
		while (Rindex1 == Rindex2) {
			/*make the elements of boats on the same route 0(testified)*/
			for (int h = 0; h < routeList.size(); h++) {
				if (routeList[h].size() != 0) {
					for (int j = 0; j < routeList[h].size() - 1; j++) {
						for (int k = j + 1; k < routeList[h].size(); k++) {
							TL[routeList[h][j] - 1][routeList[h][k] - 1] = 0;
							TL[routeList[h][k] - 1][routeList[h][j] - 1] = 0;
						}
					}
				}
				else {
					break;
				}
				
			}

			s = 0;
			for (int i = 0; i < J; i++) {
				for (int j = 0; j < J; j++) {
					s += TL[i][j];
					//cout << TL[i][j] << " ";
				}
				//cout << endl;
			}
			//cout << "sum of Tabu List: " << s << endl;

			r = randNum(s);
			//cout << "r= " << r << endl;

			for (int i = 0; i < J; i++) {
				for (int j = 0; j < J; j++) {
					r -= TL[i][j];
					if (r <= 0) {
						Bindex1 = i + 1; //i+1=B[i].Bj;
						Bindex2 = j + 1; //j+1=B[j].Bj;
						break;
					}
				}
				if (r <= 0) {
					break;
				}
			}
			Rindex1 = findRoute(routeList, Bindex1);//find out the route which ship 1 is on
			Rindex2 = findRoute(routeList, Bindex2);//find out the route which ship 2 is on
			if (Rindex1 == Rindex2) {//if ship 1 & 2 are on the samne route, rechoose...
				cout << "Two boats are in the same route. Rechoosing......" << endl;
			}
		}
		if (TL[Bindex1 - 1][Bindex2 - 1] == tl) {
			swap(routeList, Bindex1, Bindex2, Rindex1, Rindex2);
		}
		else {
			swap(routeList, Bindex1, Bindex2, Rindex1, Rindex2);
			currentSubject = calSubject(routeList, N);
			if (currentSubject < bestSoFar) {
				cout << "Find a new lower bound, lift the ban in advance." << endl;
			}
			else {
				swap(routeList, Bindex1, Bindex2, Rindex2, Rindex1);
				cout << "This operation is banned now." << endl;
				cout << "Swap boats back: " << Bindex2 << "\t" << Bindex1 << endl;
				cout << "Rechoosing......" << endl;
			}
		}
		
		Rindex1 = 0;
		Rindex2 = 0;
	} while ((currentSubject >= bestSoFar) && (TL[Bindex1 - 1][Bindex2 - 1] != tl));

	TL[Bindex1 - 1][Bindex2 - 1] = 0;
	TL[Bindex2 - 1][Bindex1 - 1] = 0;

	for (int i = 0; i < J; i++) {
		for (int j = 0; j < J; j++) {
			if (TL[i][j] < tl && i != j) {
				TL[i][j]++;
			}
			}
		}

	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	updateResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
	cout << "After swapping, new solution: " << endl;
	outputRoutes(routeList);
	outputBoatspeed(routeList);
	outputResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
}

/*Regular Tabu Search*/
void RTswap(vector<vector<int> >& routeList, int** TL, int tl, double bestSoFar, int** N, int** Q, double* Nh) {
	int s = 0;
	int r = 5;
	int Bindex1, Bindex2, Bindex0;
	int Rindex1, Rindex2, Rindex0;
	int index;
	double currentSubject = 0;
	double minSubject = 0;

	/*TP as tabu list*/
	int** TP = new int* [J];/*create rows*/
	for (int j = 0; j < J; j++) {
		TP[j] = new int[J];/*create columns*/
	}
	for (int i = 0; i < J; i++) {
		for (int j = 0; j < J; j++) {
			if (i == j) {
				TP[i][j] = 0;
			}
			else {
				TP[i][j] = tl;
			}
		}
	}

	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Swap boats(RTS)......" << endl;
	
	do {
		Bindex1 = randNum(J);
		if (Bindex1 == 0) {
			Bindex1++;
		}
		Rindex1 = findRoute(routeList, Bindex1);
		for (int h = 0; h < routeList.size(); h++) {
			for (int j = 0; j < routeList[h].size(); j++) {
				Bindex0 = routeList[h][j];
				Rindex0 = findRoute(routeList, Bindex0);
				if (Rindex1 != Rindex0) {
					swap(routeList, Bindex1, Bindex0, Rindex1, Rindex0);
					currentSubject = calSubject(routeList, N);
					if (minSubject == 0) {
						minSubject = currentSubject;
						Bindex2 = Bindex0;
					}
					else {
						if (currentSubject < minSubject) {
							minSubject = currentSubject;
							Bindex2 = Bindex0;
						}
					}
					swap(routeList, Bindex1, Bindex0, Rindex0, Rindex1);
					r--;
				}
				if (r == 0) {
					break;
				}
			}
			if (r == 0) {
				break;
			}
		}

		Rindex2 = findRoute(routeList, Bindex2);
		swap(routeList, Bindex1, Bindex2, Rindex1, Rindex2);
		currentSubject = calSubject(routeList, N);
	} while ((currentSubject >= bestSoFar) && (TL[Bindex1 - 1][Bindex2 - 1] != tl));

	TL[Bindex1 - 1][Bindex2 - 1] = 0;
	TL[Bindex2 - 1][Bindex1 - 1] = 0;

	for (int i = 0; i < J; i++) {
		for (int j = 0; j < J; j++) {
			if (TL[i][j] < tl && i != j) {
				TL[i][j]++;
			}
		}
	}

	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	updateResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
	cout << "After swapping, new solution: " << endl;
	outputRoutes(routeList);
	outputBoatspeed(routeList);
	outputResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
}


/*identify route state*/
void identifyRouteState(vector<vector<int> >& routeList, int** Q, int** N, double* Nh) {
	cout << endl;	
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Identifying Routes' Demand State......" << endl;
	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	updateN(N);
	updateQ(Q, N);
	//outputQ(Q);
	updateResidualWh(routeList, Q);
	outputResidualWh(routeList, Q);
	cout << endl;
	int Bindex;

	for (int h = 0; h < routeList.size() - 1; h++) {
		Bindex = findMinUsedBoat(R[h].Rh, routeList);
		if (R[h].residualWh > 0) {
			R[h].Sh = 0;
			//cout << "Route " << R[h].Rh << " needs more boats." << endl;
			cout << "Route " << R[h].Rh << "'s demand state: " << R[h].Sh << endl;
		}
		else if (R[h].residualWh + Q[Bindex - 1][h] <= 0) {
			R[h].Sh = 2;
			//cout << "Route " << R[h].Rh << " has extra boats." << endl;
			cout << "Route " << R[h].Rh << "'s demand state: " << R[h].Sh << endl;
		}
		else {
			R[h].Sh = 1;
			//cout << "Route " << R[h].Rh << " is good." << endl;
			cout << "Route " << R[h].Rh << "'s demand state: " << R[h].Sh << endl;
		}
	}
	updateNh(Nh);
	for (int h = 0; h < routeList.size() - 1; h++) {
		if (routeList[h].size() < Nh[h]) {
			R[h].Nh = 0;
			cout << "Route " << R[h].Rh << "'s Nh state: " << R[h].Nh << endl;
		}
		else if (routeList[h].size() == Nh[h]) {
			R[h].Nh = 1;
			cout << "Route " << R[h].Rh << "'s Nh state: " << R[h].Nh << endl;
		}
		else {
			R[h].Nh = 2;
			cout << "Route " << R[h].Rh << "'s Nh state: " << R[h].Nh << endl;
		}
	}
}

/*insert*/
void Tinsert(vector<vector<int> >& routeList, int** Q, int** N, int tl, int k) {
	cout << endl;
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Choosing a boat inserted to another route......" << endl;
	int Bindex, Rindex, rindex;
	int Rindex0, Rindex2;/*0 for underload and 2 for overload*/
	int s = 0;/*the sum of table*/
	int r;/*random number*/
	vector<int> ::iterator iter;
	int dis;
	int index;

	int** TS = new int* [J];/*create rows*/
	for (int j = 0; j < J; j++) {
		TS[j] = new int[H + 1];/*create columns*/
	}
	/*initial TS*/
	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H + 1; h++) {
			TS[j][h] = tl;
			//cout << TS[j][h] << "\t";
		}
		//cout << endl;
	}

	int a = 0, b = 0;/*the number of underload route and overload route*/
	for (int h = 0; h < routeList.size() - 1; h++) {
		if (R[h].Sh == 0) {
			a += 1;
			Rindex0 = R[h].Rh;
		}
		else if (R[h].Sh == 2) {
			b += 1;
			Rindex2 = R[h].Rh;
		}
	}

	//cout << "a = " << a << "\t" << "b = " << b << endl;

	if (a == 1 && b == 0) {
		cout << endl;
		cout << "Route " << Rindex0 << " needs more boats." << endl;
		for (int j = 0; j < routeList[H].size(); j++) {
			TS[routeList[H][j] - 1][Rindex0 - 1] *= k;
		}
	}
	else if (a == 0 && b == 1) {
		cout << endl;
		cout << "Route " << Rindex2 << " can give away a boat." << endl;
		for (int j = 0; j < J; j++) {
			TS[j][Rindex2 - 1] == 1;
		}
		Bindex = findMinUsedBoat(Rindex2, routeList);
		TS[Bindex - 1][H] *= k;
	}
	else if (a == 1 && b == 1) {
		cout << endl;
		cout << "Route " << Rindex0 << " needs more boats and route " << Rindex2 << " can give away a boat." << endl;
		Bindex = findMinUsedBoat(Rindex2, routeList);
		TS[Bindex - 1][Rindex0 - 1] *= k;
		for (int j = 0; j < J; j++) {
			TS[j][Rindex2 - 1] == 1;
		}
	}
	else if (a == 0 && b == 0) {
		cout << endl;
		cout << "Choosing randomly without any adjustment......" << endl;
	}
	else {
		for (int h = 0; h < routeList.size() - 1; h++) {
			if (R[h].Sh == 0) {
				for (int j = 0; j < routeList[H].size(); j++) {
					TS[routeList[H][j] - 1][h] *= k;
				}
			}
			else if (R[h].Sh == 2) {
				for (int j = 0; j < J; j++) {
					TS[j][h] == 1;
				}
				Bindex = findMinUsedBoat(h + 1, routeList);
				TS[Bindex - 1][H] *= k;
			}
		}
	}

	for (int h = 0; h < routeList.size(); h++) {
			for (int j = 0; j < routeList[h].size(); j++) {
				TS[routeList[h][j] - 1][h] = 0;
			}
		}

	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H + 1; h++) {
			//cout << TS[j][h] << "\t";
			s += TS[j][h];
		}
		//cout << endl;
	}
	//cout << "s= " << s << endl;
	r = randNum(s);
	//cout << "r= " << r << endl;

	for (int j = 0; j < J; j++) {
		for (int h = 0; h < H + 1; h++) {
			r -= TS[j][h];
			if (r <= 0) {
				Bindex = j + 1;
				Rindex = h + 1;
				break;
			}
		}
		if (r <= 0) {
			break;
		}
	}
	cout << "Boat " << Bindex << " is inserted to route " << Rindex << "." << endl;
	rindex = findRoute(routeList, Bindex);
	routeList[Rindex - 1].push_back(Bindex);
	iter = find(routeList[rindex - 1].begin(), routeList[rindex - 1].end(), Bindex);
	dis = std::distance(routeList[rindex - 1].begin(), iter);
	routeList[rindex - 1].erase(routeList[rindex - 1].begin() + dis);

	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	updateResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
	cout << "After inserting, new solution: " << endl;
	outputRoutes(routeList);
	outputBoatspeed(routeList);
	outputResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
}

/*adjust speed*/
void Tspeed(vector<vector<int> >& routeList, int** Q, int** N, double sd, double su, int tl, double* Nh) {
	int s = 0, c;
	int r;/*random number*/
	int Bindex = 0, sum = 0;
	int* TS = new int[J];/*container for weight of ships*/
	double gap, temp;
	//int* TB = new int[J];/*container for number of ships*/
	cout << endl;
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Adjusting boats' speed......" << endl;
	cout << "Acceleration coefficient= " << su << "\t" << "Deceleration coefficient = " << sd << endl;

	for (int h = 0; h < routeList.size() - 1; h++) {
		if (R[h].Sh == 0) {
			cout << "Boats on route " << R[h].Rh << " should speed up." << endl;
			c = 0;
			for (int j = 0; j < routeList[h].size(); j++) {
				if (B[routeList[h][j] - 1].Vj < B[routeList[h][j] - 1].maxVj) {
					c += 1;
				}
			}
			s = 0;
			while (R[h].Sh == 0 && c) {
				if (c == 0) {
					break;
				}
				int t = s % routeList[h].size();
				if (B[routeList[h][t] - 1].Vj < B[routeList[h][t] - 1].maxVj) {
					if (B[routeList[h][t] - 1].Vj * su < B[routeList[h][t] - 1].maxVj) {
						B[routeList[h][t] - 1].Vj *= su;
						updateNh(Nh);
						sum = 0;
						for (int h = 0; h < routeList.size() - 1;h++){
							sum += Nh[h];
						}
						if(sum > J){
							B[routeList[h][t] - 1].Vj /= su;
							updateNh(Nh);
							break;
						}
					}
					else if (B[routeList[h][t] - 1].Vj * su >= B[routeList[h][t] - 1].maxVj) {
						gap = B[routeList[h][t] - 1].maxVj - B[routeList[h][t] - 1].Vj;
						B[routeList[h][t] - 1].Vj = B[routeList[h][t] - 1].maxVj;
						updateNh(Nh);
						sum = 0;
						for (int h = 0; h < routeList.size() - 1;h++){
							sum += Nh[h];
						}
						if(sum > J){
							B[routeList[h][t] - 1].Vj -= gap;
							updateNh(Nh);
							break;
						}
					}
					cout << "Now the speed of boat " << routeList[h][t] << " on route " << R[h].Rh << " is " << B[routeList[h][t] - 1].Vj << "." << endl;
				}
				else if (B[routeList[h][t] - 1].Vj == B[routeList[h][t] - 1].maxVj) {
					cout << "Boat " << routeList[h][t] << "'s speed can't be adjusted." << endl;
				}
				else {
					cout << "Boat " << routeList[h][t] << "'s speed is insane." << endl;
					exit(2);
				}
				s++;							

				identifyRouteState(routeList, Q, N, Nh);
				c = 0;
				for (int j = 0; j < routeList[h].size(); j++) {
					if (B[routeList[h][j] - 1].Vj < B[routeList[h][j] - 1].maxVj) {
						c += 1;
					}
				}
			} 
			
		}
		else if (R[h].Sh == 1) {
			cout << "Boats on route " << R[h].Rh << " can decelerate a little bit." << endl;
			updateN(N);
			for (int j = 0; j < routeList[h].size(); j++) {
				gap = B[routeList[h][j] - 1].Vj;
				temp = (N[routeList[h][j] - 1][h] * R[h].Dh) / (24 * B[routeList[h][j] - 1].Tj - 24 * N[routeList[h][j] - 1][h] * Tsjh) ;
				if (temp < B[routeList[h][j] - 1].minVj) {
					B[routeList[h][j] - 1].Vj = B[routeList[h][j] - 1].minVj;
					updateNh(Nh);
					sum = 0;
					for (int h = 0; h < routeList.size() - 1; h++) {
						sum += Nh[h];
					}
					if (sum > J) {
						B[routeList[h][j] - 1].Vj = gap;
						updateNh(Nh);
						break;
					}
				}
				else if (temp > B[routeList[h][j] - 1].maxVj) {
					B[routeList[h][j] - 1].Vj = B[routeList[h][j] - 1].maxVj;
					updateNh(Nh);
					sum = 0;
					for (int h = 0; h < routeList.size() - 1; h++) {
						sum += Nh[h];
					}
					if (sum > J) {
						B[routeList[h][j] - 1].Vj = gap;
						updateNh(Nh);
						break;
					}
				}
				else {
					B[routeList[h][j] - 1].Vj = temp;
					updateNh(Nh);
					sum = 0;
					for (int h = 0; h < routeList.size() - 1; h++) {
						sum += Nh[h];
					}
					if (sum > J) {
						B[routeList[h][j] - 1].Vj = gap;
						updateNh(Nh);
						break;
					}
				}
				cout << "Now the speed of boat " << routeList[h][j] << " on route " << R[h].Rh << " speed is " << B[routeList[h][j] - 1].Vj << "." << endl;
			}
		}
		else if (R[h].Sh == 2) {
			cout << "Boats on route " << R[h].Rh << " should decelerate." << endl;
			c = 0;
			for (int j = 0; j < routeList[h].size(); j++) {
				if (B[routeList[h][j] - 1].Vj  > B[routeList[h][j] - 1].minVj) {
					c += 1;
				}
			}
			cout << "c=" << c << endl;
			s = 0;
			while (R[h].Sh == 2 && c) {
				if (c == 0) {
					break;
				}
				int t = s % routeList[h].size();
				if (B[routeList[h][t] - 1].Vj > B[routeList[h][t] - 1].minVj) {
					if (B[routeList[h][t] - 1].Vj * sd > B[routeList[h][t] - 1].minVj) {
						B[routeList[h][t] - 1].Vj *= sd;
						updateNh(Nh);
						sum = 0;
						for (int h = 0; h < routeList.size() - 1;h++){
							sum += Nh[h];
						}
						if(sum > J){
							B[routeList[h][t] - 1].Vj /= sd;
							updateNh(Nh);
							break;
						}
					}
					else if (B[routeList[h][t] - 1].Vj * sd <= B[routeList[h][t] - 1].minVj) {
						gap = B[routeList[h][t] - 1].Vj - B[routeList[h][t] - 1].minVj;
						B[routeList[h][t] - 1].Vj = B[routeList[h][t] - 1].minVj;
						updateNh(Nh);
						sum = 0;
						for (int h = 0; h < routeList.size() - 1;h++){
							sum += Nh[h];
						}
						if(sum > J){
							B[routeList[h][t] - 1].Vj += gap;
							updateNh(Nh);
							break;
						}
					}
					cout << "Now the speed of boat " << routeList[h][t] << " on route " << R[h].Rh << " is " << B[routeList[h][t] - 1].Vj << "." << endl;
				}
				else if (B[routeList[h][t] - 1].Vj == B[routeList[h][t] - 1].minVj) {
					cout << "Boat " << routeList[h][t] << "'s speed can't be adjusted." << endl;
				}
				else {
					cout << "Boat " << routeList[h][t] << "'s speed is unreasonable." << endl;
					exit(2);
				}
				s++;

				identifyRouteState(routeList, Q, N, Nh);
				if (R[h].Sh == 0) {
					B[routeList[h][t] - 1].Vj /= sd;
					cout << "Slow down too much, change boat " << routeList[h][t] << "'s speed back to " << B[routeList[h][t] - 1].Vj << "." << endl;
					break;
				}
				c = 0;
				for (int j = 0; j < routeList[h].size(); j++) {
					if (B[routeList[h][j] - 1].Vj  > B[routeList[h][j] - 1].minVj) {
						c += 1;
					}
				}
			} 
		}
	}

	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	updateResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
	cout << "After adjusting speed, the route solution is: " << endl;
	outputRoutes(routeList);
	outputBoatspeed(routeList);
	outputResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
}

void addShipsFromOtherRoute(vector<vector<int> >& routeList, int** Q, int** N, double* Nh, int Rindex) {
	bool s = 1;
	cout << "Route " << Rindex << " needs more boats for operation. Try to find boat on other route......" << endl;
	while (routeList[Rindex - 1].size() < Nh[Rindex - 1]) {
		for (int h = 0; h < routeList.size() - 1; h++) {
			if (routeList[h].size() > Nh[h] && h != Rindex - 1) {
				cout << "Route " << R[h].Rh << " has extra ships." << endl;
				add(R[h].Rh, Rindex, routeList);
			}
		}
	}
}

/*add & delete*/
void addDelete(vector<vector<int> >& routeList, int** Q, int** N, double* Nh) {
	int Bindex;
	vector<int> ::iterator iter;
	int dis;
	cout << endl;
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Now Add&Delete......" << endl;

	for (int h = 0; h < routeList.size() - 1; h++) {
		if(R[h].Sh == 0 || routeList[h].size() < Nh[h]) {
			cout << "Route " << R[h].Rh << " needs more boats, idle boats first." << endl;
			while (R[h].Sh == 0 || routeList[h].size() < Nh[h]) {
				Bindex = findMinIdleBoat(R[h].Rh, routeList);
				/*In case there is no idle ships*/
				if (Bindex == -1) {
					addShipsFromOtherRoute(routeList, Q, N, Nh, R[h].Rh);
					break;
				}
				else {
					cout << "Add boat " << Bindex << " to route " << R[h].Rh << endl;
					routeList[h].push_back(Bindex);
					iter = find(routeList[H].begin(), routeList[H].end(), Bindex);
					dis = std::distance(routeList[H].begin(), iter);
					routeList[H].erase(routeList[H].begin() + dis);

					identifyRouteState(routeList, Q, N, Nh);
				}
			}
		}
		else if (R[h].Sh == 1 && routeList[h].size() < Nh[h]) {
			cout << "Route " << R[h].Rh << " needs more boats, idle boats first." << endl;
			while (R[h].Sh == 1 && routeList[h].size() < Nh[h]) {
				Bindex = findMinIdleBoat(R[h].Rh, routeList);
				/*In case there is no idle ships*/
				if (Bindex == -1) {
					addShipsFromOtherRoute(routeList, Q, N, Nh, h + 1);
				}
				else {
					cout << "Add boat " << Bindex << " to route " << R[h].Rh << endl;
					routeList[h].push_back(Bindex);
					iter = find(routeList[H].begin(), routeList[H].end(), Bindex);
					dis = std::distance(routeList[H].begin(), iter);
					routeList[H].erase(routeList[H].begin() + dis);

					identifyRouteState(routeList, Q, N, Nh);
				}
			}
		}
		else if (R[h].Sh == 2 && routeList[h].size()>Nh[h]) {
			cout << "Route " << R[h].Rh << " has extra boats." << endl;
			while (R[h].Sh == 2 && routeList[h].size() > Nh[h]) {
				Bindex = findMinUsedBoat(R[h].Rh, routeList);
				cout << "Add boat " << Bindex << " to idle route.";
				routeList[H].push_back(Bindex);
				iter = find(routeList[h].begin(), routeList[h].end(), Bindex);
				dis = std::distance(routeList[h].begin(), iter);
				routeList[h].erase(routeList[h].begin() + dis);

				identifyRouteState(routeList, Q, N, Nh);
			}
		}
	}

	for (int h = 0; h < routeList.size() - 1; h++) {
		R[h].residualInit();
	}
	updateResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
	cout << "After add&delete, new solution: " << endl;
	outputRoutes(routeList);
	outputBoatspeed(routeList);
	outputResidualWh(routeList, Q);
	cout << "********************************************************************************" << endl;
}

bool isFeasibility(double* Nh, vector<vector<int> >& routeList, int** Q, int** N) {
	bool fea = 1;
	identifyRouteState(routeList, Q, N, Nh);
	for (int h = 0; h < routeList.size() - 1; h++) {
		if (R[h].Sh == 0) {
			fea = 0;
			cout << "Route " << R[h].Rh << " does not meet its demand." << endl;
		}
		else if (R[h].Nh == 0) {
			fea = 0;
			cout << "Route " << R[h].Rh << " does not have enough ships to run the schedule." << endl;
		}
	}
	return fea;
}
















int main() {
/*---------------------------------Timer Starter---------------------------------*/
	clock_t startTime, endTime;
	startTime = clock();

/*---------------------------------Preparations---------------------------------*/
	/*input data*/
	cout << "Inputing Data......" << endl << endl;
	J = CountItemNumbers("boats.txt");
	H = CountItemNumbers("routes.txt");
	io_boats();
	io_routes();
	cout << "Data input finished!" << endl << endl;
	/*paraemters setting*/
	int tl = 5;/*tabu Length*/
	int k = 5;/*weight increase multiple*/
	double sd = 0.8, su = 1.2;/*speed increase coefficient and decrease coefficient*/
	int p = 0, P = 320;/*search times,iterations*/

	/*containers for solution and subject*/
	double bestCO = 0, bestCP = 0, bestCR = 0, bestM = 0, bestO = 0;
	double averUsedSpeed = 0, averIdleSpeed = 0, averAllSpeed = 0;
	bestB = new boats[J];
	bestR = new routes[H];
	vector<vector<int> > routeList(H + 1);
	vector<vector<int> > bestRoutes(H + 1);/*best solution*/

	double currentSubject = 0;/*current subject*/
	double bestSoFar = 0;/*Best subject so far*/
	double* Nh = new double[H];/*the minimum numbers of ships on routes*/
	double* rWh = new double[H];/*residual demand of routes*/
	double* bestrWh = new double[H];/*residual demand of routes of best solution*/
	double* bestV = new double[J];/*speed of boats of best solution*/

	/*containers for matrix N and Q*/
	int** N = new int* [J];/*create rows*/
	for (int j = 0; j < J; j++) {
		N[j] = new int[H];/*create columns*/
	}

	int** Q = new int* [J];/*create rows*/
	for (int j = 0; j < J; j++) {
		Q[j] = new int[H];/*create columns*/
	}

	/*tabu list*/
	int** TL = new int* [J];/*create rows*/
	for (int j = 0; j < J; j++) {
		TL[j] = new int[J];/*create columns*/
	}

	/*random number*/
	int r;

/*---------------------------------construct initial solution---------------------------------*/
	initRouteLists(routeList);/*initial route list(including idle route)*/
	initTL(TL, tl);/*initial TS swap table*/
	initAssignment(routeList, Q, N);
	outputRoutes(routeList);
	updateNh(Nh);
	updateRouteNhState(Nh, routeList);
	for (int h = 0; h < routeList.size() - 1;h++){
		if(R[h].Nh==0){
			//cout << "Route "<<R[h].Rh<<" needs more ships to operate.Nh adjusting......" << endl;
			NhAdjust(Nh, routeList, R[h].Rh);
			if (routeList[h].size() >= Nh[h]) {
				cout << "Route " << R[h].Rh << " Nh adjxustment done." << endl;
			}
			else {
				cout << "Need more ships." << endl;
			}
		}
	}
	/*updateRouteNhState(Nh, routeList);
	for (int h = 0; h < routeList.size() - 1;h++){
		if(R[h].Nh==0){
			addShipsFromOtherRoute(routeList, Q, N, Nh, R[h].Rh);
		}
		if(routeList[h].size()>=Nh[h]){
			cout << "Route " << R[h].Rh << " Nh adjustment done." << endl;
		}
	}*/

	fea = isFeasibility(Nh, routeList, Q, N);

	if (fea) {
		cout << "********************************************************************************" << endl;
		cout << "The initial route solution is:" << endl;
		outputRoutes(routeList);
		outputBoatspeed(routeList);
		outputResidualWh(routeList, Q);
		cout << "********************************************************************************" << endl;
		currentSubject = calSubject(routeList, N);
		for (int h = 0; h < H; h++) {
			rWh[h] = R[h].residualWh;
		}
		bestSoFar = currentSubject;
		bestB = B;
		bestR = R;
		bestRoutes = routeList;
	}
	else {
		cout << "Program can't make the initial solution. The problem is infeasible." << endl;
		system("pause");
	}

/*---------------------------------PTS Test---------------------------------*/
	


/*---------------------------------Probabilistic Tabu Search Iterations---------------------------------*/
	while(p<P){
		fea = 0;
		cout << endl << endl;
		cout << "---------------------------------------Iteration: " << p + 1 << "---------------------------------------" << endl;
		PTswap(routeList, TL, tl, bestSoFar, N, Q, Nh);
		identifyRouteState(routeList, Q, N, Nh);
		Tinsert(routeList, Q, N, tl, k);
		identifyRouteState(routeList, Q, N, Nh);
		addDelete(routeList, Q, N, Nh);
		Tspeed(routeList, Q, N, sd, su, tl, Nh);
		updateN(N);
		updateQ(Q, N);
		
		updateNh(Nh);
		updateRouteNhState(Nh, routeList);
		for (int h = 0; h < routeList.size() - 1;h++){
			if(R[h].Nh==0){
				NhAdjust(Nh, routeList, R[h].Rh);
			}
			if(routeList[h].size()>=Nh[h]){
			cout << "Route " << R[h].Rh << " Nh adjustment done." << endl;
			}
		}
		updateRouteNhState(Nh, routeList);
		for (int h = 0; h < routeList.size() - 1;h++){
			if(R[h].Nh==0){
				addShipsFromOtherRoute(routeList, Q, N, Nh, R[h].Rh);
			}
			if(routeList[h].size()>=Nh[h]){
			cout << "Route " << R[h].Rh << " Nh adjustment done." << endl;
			}
		}

		fea = isFeasibility(Nh, routeList, Q, N);
		if (fea) {
			cout << "********************************************************************************" << endl;
			cout << "After the iteration " << p + 1 << ", the current routelist:" << endl;
			outputRoutes(routeList);
			outputBoatspeed(routeList);
			outputResidualWh(routeList, Q);
			cout << "********************************************************************************" << endl;

			currentSubject = calSubject(routeList, N);
			for (int h = 0; h < H; h++) {
				rWh[h] = R[h].residualWh;
			}

			if ((currentSubject < bestSoFar) && fea) {
				cout << "Find a better solution!" << endl;
				bestCO = 0;
				bestCP = 0;
				bestCR = 0;
				bestM = 0;
				bestO = 0;
				bestSoFar = currentSubject;
				bestRoutes = routeList;
				bestCO = updateCO(bestRoutes, N);
				bestCP = updateCP(bestRoutes, N);
				for (int h = 0; h < routeList.size() - 1; h++) {
					for (int j = 0; j < routeList[h].size(); j++) {
						bestM += B[routeList[h][j] - 1].Mj;
						bestCR += CRjh;
					}
				}
				for (int j = 0; j < routeList[H].size(); j++) {
					bestO += B[routeList[H][j] - 1].Fj;
				}

				for (int h = 0; h < H; h++) {
					bestrWh[h] = rWh[h];
					//cout << bestrWh[h];
				}
				bestB = B;
				bestR = R;
			}
		}
		else {
			cout << "After the iteration " << p + 1 << ", the solution is now infeasible. Please try again." << endl;
			system("pause");
		}
		p++;
		//system("pause");
	}

/*---------------------------------Output results---------------------------------*/
	cout << endl;
	cout << "Iteration Complete!" << endl;
	cout << "********************************************************************************" << endl;
	cout << "Best solution:" << endl;
	outputRoutes(bestRoutes);
	cout << "Base on the best solution:" << endl;
	cout << "Operation costs: " << bestCR << endl;
	cout << "Oil costs: " << bestCO << endl;
	cout << "Port costs: " << bestCP << endl;
	cout << "Rent costs: " << bestM << endl;
	cout << "Idle costs:" << bestO << endl;
	cout << "Best subject:" << bestSoFar << endl;
	bestB = B;
	cout << "Boats speed:";
	for (int j = 0; j < J; j++) {
		if (j % 5 == 0) {
			cout << endl;
		}
		cout << bestB[j].Vj << "\t";
	}
	for (int j = 0; j < J; j++) {
		averAllSpeed += bestB[j].Vj;
	}
	averAllSpeed /=  J;
	for (int h = 0; h < bestRoutes.size() - 1; h++) {
		for (int j = 0; j < bestRoutes[h].size(); j++) {
			averUsedSpeed += bestB[bestRoutes[h][j] - 1].Vj;
		}
	}
	averUsedSpeed /= (J - bestRoutes[H].size());
	cout << endl;
	cout << "There are " << J - bestRoutes[H].size() << " ships in use." << endl;
	cout << "Average speed of all ships assigned: " << averUsedSpeed << endl;
	cout << "Average speed of all ships: " << averAllSpeed << endl;
	for (int h = 0; h < H; h++) {
		// cout << "The residual freight denmand of route " << R[h].Rh << " is: " << bestrWh[h] << endl;
		cout << "The residual freight denmand of route " << R[h].Rh << " is: " << bestR[h].residualWh << endl;
	}
	cout << "********************************************************************************" << endl;

/*---------------------------------Timer end---------------------------------*/
	endTime = clock();
	cout << endl;
	cout << "Totle Time : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	//system("time");
	return 0;
}