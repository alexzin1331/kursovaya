#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace std;

// Класс для автомобиля
class Car {
public:
    int id;
    time_t arrivalTime;
    int waitingTime;
    bool isEmergency;

    Car(int carId, bool emergency = false) : id(carId), arrivalTime(time(nullptr)), waitingTime(0), isEmergency(emergency) {}
};

// Класс для представления полосы движения
class Lane {
private:
    queue<Car*> cars;
    int maxSize;
    string direction;
    bool isTurnLane;

public:
    Lane(int size, string dir, bool turn = false) : maxSize(size), direction(dir), isTurnLane(turn) {}

    bool addCar(Car* car) {
        if (cars.size() < maxSize) {
            cars.push(car);
            return true;
        }
        return false;
    }

    Car* removeCar() {
        if (!cars.empty()) {
            Car* frontCar = cars.front();
            cars.pop();
            return frontCar;
        }
        return nullptr;
    }

    bool isEmpty() const {
        return cars.empty();
    }

    int getCarCount() const {
        return cars.size();
    }

    string getDirection() const {
        return direction;
    }

    bool isTurn() const {
        return isTurnLane;
    }

    void updateWaitingTimes() {
        queue<Car*> temp;
        while (!cars.empty()) {
            Car* car = cars.front();
            car->waitingTime++;
            temp.push(car);
            cars.pop();
        }
        cars = temp;
    }
};

// Класс для перекрестка
class Intersection {
private:
    int id;
    vector<Lane*> lanes;
    bool trafficLight; // true - зеленый, false - красный
    int lightDuration;
    int lightCounter;
    int totalCarsPassed;
    int totalWaitingTime;
    int idleCars;

public:
    Intersection(int intersectionId, int lightDur) : id(intersectionId), trafficLight(true), 
                                                  lightDuration(5), lightCounter(0),
                                                  totalCarsPassed(0), totalWaitingTime(0),
                                                  idleCars(0) {
        // Создаем 4 прямые полосы (N, S, E, W)
        lanes.push_back(new Lane(10, "North"));
        lanes.push_back(new Lane(10, "South"));
        lanes.push_back(new Lane(10, "East"));
        lanes.push_back(new Lane(10, "West"));
        
        // Создаем 4 полосы для поворотов
        lanes.push_back(new Lane(5, "North", true));
        lanes.push_back(new Lane(5, "South", true));
        lanes.push_back(new Lane(5, "East", true));
        lanes.push_back(new Lane(5, "West", true));
    }

    ~Intersection() {
        for (auto lane : lanes) {
            delete lane;
        }
    }

    void update() {
        // Обновление светофора
        lightCounter++;
        if (lightCounter >= lightDuration) {
            trafficLight = !trafficLight;
            lightCounter = 0;
        }

        // Обновление времени ожидания для всех машин
        for (auto lane : lanes) {
            lane->updateWaitingTimes();
        }

        // Обработка движения машин если светофор зеленый
        if (trafficLight) {
            processMovement();
        }
    }

    void processMovement() {
        // Обрабатываем прямые полосы
        for (int i = 0; i < 4; ++i) {
            if (!lanes[i]->isEmpty()) {
                Car* car = lanes[i]->removeCar();
                totalCarsPassed++;
                totalWaitingTime += car->waitingTime;
                if (car->waitingTime > 5) idleCars++;
                delete car;
            }
        }

        // Обрабатываем полосы для поворотов (используем стек)
        stack<Car*> turnCars;
        for (int i = 4; i < 8; ++i) {
            if (!lanes[i]->isEmpty()) {
                turnCars.push(lanes[i]->removeCar());
            }
        }

        // Разбираем стек с поворачивающими машинами
        while (!turnCars.empty()) {
            Car* car = turnCars.top();
            totalCarsPassed++;
            totalWaitingTime += car->waitingTime;
            if (car->waitingTime > 5) idleCars++;
            delete car;
            turnCars.pop();
        }
    }

    void addCar(int direction, bool isTurn = false, bool emergency = false) {
        int laneIndex = direction + (isTurn ? 4 : 0);
        if (laneIndex >= 0 && laneIndex < lanes.size()) {
            Car* car = new Car(rand() % 10000, emergency);
            if (!lanes[laneIndex]->addCar(car)) {
                delete car; // Если полоса заполнена, удаляем машину
            }
        }
    }

    void displayStatus(int currentTime) const {
        cout << "=== Перекресток " << id << " ===" << endl;
        cout << "Время: " << currentTime << " сек" << endl;
        cout << "Светофор: " << (trafficLight ? "ЗЕЛЕНЫЙ" : "КРАСНЫЙ") << endl;
        
        cout << "Прямые полосы:" << endl;
        for (int i = 0; i < 4; ++i) {
            cout << lanes[i]->getDirection() << ": " << lanes[i]->getCarCount() << " машин" << endl;
        }
        
        cout << "Полосы для поворотов:" << endl;
        for (int i = 4; i < 8; ++i) {
            cout << lanes[i]->getDirection() << ": " << lanes[i]->getCarCount() << " машин" << endl;
        }
        cout << endl;
    }

    void generateReport() const {
        cout << "=== Отчет по перекрестку " << id << " ===" << endl;
        cout << "Всего машин проехало: " << totalCarsPassed << endl;
        if (totalCarsPassed > 0) {
            cout << "Среднее время ожидания: " 
                 << fixed << setprecision(2) 
                 << (double)totalWaitingTime / totalCarsPassed << " сек" << endl;
        } else {
            cout << "Среднее время ожидания: 0 сек" << endl;
        }
        cout << "Количество простаивавших машин (>5 сек): " << idleCars << endl;
        cout << endl;
    }
};

// Класс для управления симуляцией
class Simulation {
private:
    vector<Intersection*> intersections;
    int simulationTime;
    int carSpawnRate;

public:
    Simulation(int intersectionsCount, int simTime, int spawnRate) : 
              simulationTime(simTime), carSpawnRate(spawnRate) {
        for (int i = 0; i < intersectionsCount; ++i) {
            intersections.push_back(new Intersection(i + 1, 10 + rand() % 10));
        }
    }

    ~Simulation() {
        for (auto intersection : intersections) {
            delete intersection;
        }
    }

    void run() {
        for (auto intersection : intersections) {
            for (int i = 0; i < 5; ++i) {
                intersection->addCar(rand() % 4, rand() % 4 == 0);
            }
        }

        for (int time = 0; time < simulationTime; ++time) {
            if (time % carSpawnRate == 0) {
                for (auto intersection : intersections) {
                    int direction = rand() % 4;
                    bool isTurn = rand() % 4 == 0; // (25% вероятность поворота)
                    bool emergency = rand() % 20 == 0; // (5% вероятность, что проедет спецтранспорт)
                    intersection->addCar(direction, isTurn, emergency);
                }
            }

            // Обновляем перекрестки
            for (auto intersection : intersections) {
                intersection->update();
                
                //каждые 2 секунды выводим то, что происходит
                if (time % 2 == 0) {
                    intersection->displayStatus(time);
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
            }

            // Пауза для визуализации
            this_thread::sleep_for(chrono::seconds(1));
        }

        // Генерация отчетов
        cout << "\n=== Итоговые отчеты ===" << endl;
        for (auto intersection : intersections) {
            intersection->generateReport();
        }
    }
};

int main() {
    srand(time(nullptr)); // Инициализация генератора случайных чисел
    
    cout << "Симуляция транспортной сети" << endl;
    cout << "Введите количество перекрестков: ";
    int intersections;
    cin >> intersections;
    
    cout << "Введите время симуляции (сек): ";
    int simTime;
    cin >> simTime;
    
    cout << "Введите частоту появления машин (сек): ";
    int spawnRate;
    cin >> spawnRate;
    
    Simulation sim(intersections, simTime, spawnRate);
    sim.run();
    
    return 0;
}