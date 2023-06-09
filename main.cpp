#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>
#include <random>
#include <future>
using namespace std;

void sumar_rango(vector<int>& vdata, int& resultado, int start, int stop) {
    auto it_start = next(begin(vdata), start);
    auto it_stop = next(begin(vdata), stop);
    resultado = accumulate(it_start, it_stop, 0);
}

void ejemplo_sumar_vector() {
    vector v = {1,-10,5,11,7,9,20,50,11,14,-17,8,5,2,33,3};
    int nh = 4;
    auto rango = v.size() / nh;
    // 1. Crear el vector de hilos
    vector<thread> vh(nh); // Vector de hilos de tamaño nh
    // Crear el vector de resultados
    vector<int> vr(nh);  // de tamaño nh
    // 2. Asignar una tarea o funcion a los hilos
    for (int j = 0; j < nh; ++j) {
        vh[j] = thread(sumar_rango, ref(v), ref(vr[j]),
                       j * rango,
                       (j+1) * rango);
    }
     // 3. Union de los hilos
    for (int j = 0; j < nh; ++j) {
        vh[j].join();
    }

    auto total_sh = accumulate(begin(v), end(v), 0);
    auto total_ch = accumulate(begin(vr), end(vr), 0);
    cout << total_sh  << " - " << total_ch << endl;
}

template <typename Iterator>
void sumar_rango_iter(int& resultado, Iterator start, Iterator stop) {
    resultado = accumulate(start, stop, 0);
}

auto acumular_paralelo(vector<int>::iterator start,
                       vector<int>::iterator stop) {
    int nh = static_cast<int>(thread::hardware_concurrency());
    int rango = static_cast<int>(distance(start, stop) / nh);
    // 1. Crear el vector de hilos
    vector<thread> vh(nh); // Vector de hilos de tamaño nh
    // Crear el vector de resultados
    vector<int> vr(nh);  // de tamaño nh
    // 2. Asignar una tarea o funcion a los hilos
    for (int j = 0; j < nh; ++j) {
        auto it_start = next(start, j * rango);
        auto it_stop = next(start, (j + 1)* rango);
        vh[j] = thread(sumar_rango_iter<vector<int>::iterator>,
                       ref(vr[j]),
                       it_start,
                       it_stop);
    }
    // 3. Union de los hilos
    for (int j = 0; j < nh; ++j) {
        vh[j].join();
    }
    return accumulate(begin(vr), end(vr), 0);
}

auto acumular_paralelo_future(vector<int>::iterator start,
                       vector<int>::iterator stop) {
    int nf = static_cast<int>(thread::hardware_concurrency());
    int rango = static_cast<int>(distance(start, stop) / nf);
    // 1. Crear el vector de futures
    vector<future<int>> vf(nf);
    // 2. Asignar una tarea o funcion a los futures
    for (int j = 0; j < nf; ++j) {
        auto it_start = next(start, j * rango);
        auto it_stop = next(start, (j + 1)* rango);
        vf[j] = async(
                [it_start, it_stop]{
                    return accumulate(it_start, it_stop, 0);
                });
    }
    return accumulate(begin(vf), end(vf), 0,
                      [](auto sum, auto& item){
        sum += item.get();
        return sum;
    });
}

void ejemplo_2() {
    vector<int> v(1000000 * thread::hardware_concurrency());
    random_device rd;
    uniform_int_distribution<int> uid(1, 20);

    for (auto& item: v)
        item = uid(rd);

    using namespace std::chrono;

    auto start = high_resolution_clock::now();
    auto total = acumular_paralelo(begin(v), end(v));
    auto stop = high_resolution_clock::now();
    cout << "Total C/H: " <<  total << " "
        << duration_cast<microseconds>(stop - start).count()
        << endl;
    start = high_resolution_clock::now();
    auto total_sh = accumulate(begin(v), end(v), 0);
    stop = high_resolution_clock::now();
    cout << "Total S/H: " <<  total_sh  << " "
         << duration_cast<microseconds>(stop - start).count()
         << endl;
    start = high_resolution_clock::now();
    auto total_ca = acumular_paralelo_future(begin(v), end(v));
    stop = high_resolution_clock::now();
    cout << "Total C/A: " <<  total_ca  << " "
         << duration_cast<microseconds>(stop - start).count()
         << endl;
}

int main() {
//    ejemplo_sumar_vector();
//    cout << thread::hardware_concurrency() << endl;
    ejemplo_2();
    return 0;
}
