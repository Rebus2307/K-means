#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>

using namespace std;

struct Pixel {
    unsigned char r, g, b;
};

struct Point {
    float x, y, z;
};

int main() {
    // Cargar la imagen PGM en formato binario
    const char* filename = "imagen.pgm";
    ifstream infile(filename, ios::binary);

    if (!infile) {
        cerr << "No se pudo abrir la imagen." << endl;
        return -1;
    }

    // Leer el encabezado de la imagen PGM
    string format;
    int width, height, max_value;
    infile >> format >> width >> height >> max_value;

    // Leer los píxeles de la imagen
    vector<Pixel> pixels(width * height);
    infile.read(reinterpret_cast<char*>(pixels.data()), pixels.size() * sizeof(Pixel));
    infile.close();

    // Reshape la imagen para trabajar con los datos como un conjunto de puntos 3D (r, g, b)
    vector<Point> points(width * height);
    for (int i = 0; i < width * height; ++i) {
        points[i].x = pixels[i].r;
        points[i].y = pixels[i].g;
        points[i].z = pixels[i].b;
    }

    // Definir el número de clusters (en este caso, 2 para la segmentación en dos colores)
    int k = 2;

    // Inicializar los centroides de manera aleatoria
    vector<Point> centroids(k);
    for (int i = 0; i < k; ++i) {
        centroids[i] = points[rand() % (width * height)];
    }

    // Configurar el número máximo de iteraciones
    int max_iterations = 100;

    // Aplicar el algoritmo k-means
    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        // Asignar cada punto al centroide más cercano
        vector<vector<Point>> clusters(k);
        for (const auto& point : points) {
            int closest_centroid = 0;
            float min_distance = numeric_limits<float>::max();

            for (int i = 0; i < k; ++i) {
                float distance = sqrt(pow(point.x - centroids[i].x, 2) +
                                      pow(point.y - centroids[i].y, 2) +
                                      pow(point.z - centroids[i].z, 2));

                if (distance < min_distance) {
                    min_distance = distance;
                    closest_centroid = i;
                }
            }

            clusters[closest_centroid].push_back(point);
        }

        // Calcular nuevos centroides
        for (int i = 0; i < k; ++i) {
            if (!clusters[i].empty()) {
                centroids[i].x = 0;
                centroids[i].y = 0;
                centroids[i].z = 0;

                for (const auto& point : clusters[i]) {
                    centroids[i].x += point.x;
                    centroids[i].y += point.y;
                    centroids[i].z += point.z;
                }

                centroids[i].x /= clusters[i].size();
                centroids[i].y /= clusters[i].size();
                centroids[i].z /= clusters[i].size();
            }
        }

        // Salida para depuración
        cout << "Iteraciones: " << iteration + 1 << endl;
        cout << "Centroides: ";
        for (const auto& centroid : centroids) {
            cout << "(" << centroid.x << ", " << centroid.y << ", " << centroid.z << ") ";
        }
        cout << endl;

        // Verificar convergencia
        bool converged = true;
        for (int i = 0; i < k; ++i) {
            if (centroids[i].x != centroids[i].x || centroids[i].y != centroids[i].y || centroids[i].z != centroids[i].z) {
                converged = false;
                break;
            }
        }

        if (converged) {
            cout << "Convergencia despues de " << iteration + 1 << " iteraciones." << endl;
            break;
        }
    }

    // Asignar colores a cada cluster en la imagen original
    for (int i = 0; i < width * height; ++i) {
        int closest_centroid = 0;
        float min_distance = numeric_limits<float>::max();

        for (int j = 0; j < k; ++j) {
            float distance = sqrt(pow(points[i].x - centroids[j].x, 2) +
                                  pow(points[i].y - centroids[j].y, 2) +
                                  pow(points[i].z - centroids[j].z, 2));

            if (distance < min_distance) {
                min_distance = distance;
                closest_centroid = j;
            }
        }

        // Asignar colores específicos a cada cluster
        if (closest_centroid == 0) {
            pixels[i].r = 255; // Rojo
            pixels[i].g = 0;
            pixels[i].b = 0;
        } else {
            pixels[i].r = 0;
            pixels[i].g = 255; // Verde
            pixels[i].b = 0;
        }
    }

    // Guardar la imagen segmentada
    ofstream outfile("imagen_segmentada.ppm", ios::binary);
    if (!outfile) {
        cerr << "No se pudo guardar la imagen segmentada." << endl;
        return -1;
    }

    outfile << "P6\n" << width << " " << height << "\n" << max_value << "\n";
    outfile.write(reinterpret_cast<const char*>(pixels.data()), pixels.size() * sizeof(Pixel));
    outfile.close();

    return 0;
}
