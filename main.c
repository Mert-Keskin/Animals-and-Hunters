#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

typedef enum { BEAR, BIRD, PANDA} AnimalType;

typedef enum { ALIVE, DEAD } AnimalStatus;

typedef struct {
    int x;
    int y;
} Location;

typedef enum { FEEDING, NESTING, WINTERING } SiteType;

typedef struct {
    /** animal ölü yada canlı olabilir*/
    AnimalStatus status;
    /** animal type, bear, bird, panda*/
    AnimalType type;
    /** 2 boyutta konumu*/
    Location location;
} Animal;

/*örnek kullanım*/
Animal bird, bear, panda;

typedef struct {
    /** hunterin öldürdügü hayvan sayısı*/
    int points;
    Location location;
} Hunter;

typedef struct {
    /** alandaki hunterları gosteren pointer arrayi*/
    Hunter **hunters;
    /** alandaki hunter sayısı*/
    int nhunters;
    /** alandaki animalları gosteren pointer arrayi*/
    Animal **animals;
    /** alandaki hayvan sayısı*/
    int nanimals;
    /** alanın tipi*/
    SiteType type;
} Site;

/** 2 boyutlu alan*/
typedef struct {
    int xlength;
    int ylength;
    Site **sites;
} Grid;

Grid grid = {0, 0, NULL};

//rastgele tiplere gore alanı oluşturur
Grid initgrid(int xlength, int ylength) {
    grid.xlength = xlength;
    grid.ylength = ylength;

    grid.sites = (Site **)malloc(sizeof(Site *) * xlength);
    for (int i = 0; i < xlength; i++) {
        grid.sites[i] = (Site *)malloc(sizeof(Site) * ylength);
        for (int j = 0; j < ylength; j++) {
            grid.sites[i][j].animals = NULL;
            grid.sites[i][j].hunters = NULL;
            grid.sites[i][j].nhunters = 0;
            grid.sites[i][j].nanimals = 0;
            double r = rand() / (double)RAND_MAX;
            SiteType st;
            if (r < 0.33)
                st = WINTERING;
            else if (r < 0.66)
                st = FEEDING;
            else
                st = NESTING;
            grid.sites[i][j].type = st;
        }
    }

    return grid;
}


void deletegrid() {
    for (int i = 0; i < grid.xlength; i++) {
        free(grid.sites[i]);
    }

    free(grid.sites);

    grid.sites = NULL;
    grid.xlength = -1;
    grid.ylength = -1;
}

//animal ve hunter sayısını yazdırır.
void printgrid() {
    for (int i = 0; i < grid.xlength; i++) {
        for (int j = 0; j < grid.ylength; j++) {
            Site *site = &grid.sites[i][j];
            int count[3] = {0}; /* do not forget to initialize*/
            for (int a = 0; a < site->nanimals; a++) {
                Animal *animal = site->animals[a];
                count[animal->type]++;
            }

            printf("|%d-{%d, %d, %d}{%d}|", site->type, count[0], count[1],
                   count[2], site->nhunters);
        }
        printf("\n");
    }
}

//istelinen siteın bilgisini yazdırır
void printsite(Site *site) {
    int count[3] = {0};
    for (int a = 0; a < site->nanimals; a++) {
        Animal *animal = site->animals[a];
        count[animal->type]++;
    }
    printf("|%d-{%d,%d,%d}{%d}|", site->type, count[0], count[1], count[2],
           site->nhunters);
}


pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;// animals için
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;// hunters için

void remove_animal(Site *site, Animal *animal) {
    //for dongusu ile animalın indeksi bulunur.
    int animal_index = -1;
    for (int i = 0; i < site->nanimals; i++) {
        if (site->animals[i] == animal) {
            animal_index = i;
            break;
        }
    }

    if (animal_index != -1) {
        //site->animals daki son animal kaldırmak istedigimiz indexe yerleşir
        pthread_mutex_lock(&m1);
        site->animals[animal_index] = site->animals[site->nanimals-1];
        site->nanimals--;
        pthread_mutex_unlock(&m1);
    }
}

//remove_animal fonksiyonu ile çok benzerdir.
void remove_hunter(Site *site, Hunter *hunter) {
    //for dongusu ile hunterın indeksi bulunur.
    int hunter_index = -1;
    for (int i = 0; i < site->nhunters; i++) {
        if (site->hunters[i] == hunter) {
            hunter_index = i;
            break;
        }
    }

    if (hunter_index != -1) {
        //site->hunters daki son hunter kaldırmak istedigimiz indexe yerleşir
        pthread_mutex_lock(&m2);
        site->hunters[hunter_index] = site->hunters[site->nhunters-1];
        site->nhunters--;
        pthread_mutex_unlock(&m2);
    }
}

void add_animal(Site *site, Animal *animal) {
    pthread_mutex_lock(&m1);
    site->nanimals++;
    site->animals[site->nanimals - 1] = animal;
    pthread_mutex_unlock(&m1);
}

void add_hunter(Site *site, Hunter *hunter) {
    pthread_mutex_lock(&m2);
    site->nhunters++;
    site->hunters[site->nhunters - 1] = hunter;
    pthread_mutex_unlock(&m2);
}

Location random_neighbor(Location location) {
    Location neighbor;
    do {
        neighbor.x = location.x + (rand() % 3) - 1;
        neighbor.y = location.y + (rand() % 3) - 1;
    } while (neighbor.x < 0 || neighbor.x >= grid.xlength || neighbor.y < 0 || neighbor.y >= grid.ylength);

    return neighbor;
}

//hunter yada animalı rastgele hareket ettirir.
void *simulateanimal(void *args) {
    Animal *animal = (Animal *)args;
    while (1) {
        if (animal->status == DEAD) {
            pthread_exit(NULL);
        }
        Site *site = &grid.sites[animal->location.x][animal->location.y];
        switch (site->type) {
            case FEEDING:
                if ((rand()%5)< 4) {
                    continue;
                }
                else{
                    // rastgele komşu lokasyona gider
                    Location new_location = random_neighbor(animal->location);
                    remove_animal(site, animal);
                    animal->location = new_location;
                    add_animal(&grid.sites[new_location.x][new_location.y], animal);
                }
                break;

            case NESTING:
                // ürer.
                Animal *new_animal = (Animal *)malloc(sizeof(Animal));
                new_animal->type = animal->type;
                new_animal->status = ALIVE;
                new_animal->location = animal->location;
                pthread_t thread_id;
                pthread_create(&thread_id, NULL, simulateanimal, (void *)new_animal);
                add_animal(site, new_animal);

                // rastgele komşu lokasyona gider
                Location new_location = random_neighbor(animal->location);
                remove_animal(site, animal);
                animal->location = new_location;
                add_animal(&grid.sites[new_location.x][new_location.y], animal);
                break;

            case WINTERING:
                // ölür.
                if ((rand()%2) < 1) {
                    animal->status = DEAD;
                    remove_animal(site, animal);
                    pthread_exit(NULL);
                }
                else{
                    // rastgele komşu lokasyona gider
                    Location new_location = random_neighbor(animal->location);
                    remove_animal(site, animal);
                    animal->location = new_location;
                    add_animal(&grid.sites[new_location.x][new_location.y], animal);
                }
                break;
        }
        usleep(1000);  // 1 milisaniye bekler.
    }
    return NULL;
}

// hunterin hareketi simule edilir
void *simulatehunter(void *args) {
    Hunter *hunter = (Hunter *)args;
    while (1) {
        // rastgele komşu lokasyona gider
        Site *site = &grid.sites[hunter->location.x][hunter->location.y];
        remove_hunter(site, hunter);
        Location new_location = random_neighbor(hunter->location);
        hunter->location = new_location;
        hunter->points += site->nanimals;
        site = &grid.sites[hunter->location.x][hunter->location.y];
        add_hunter(site,hunter);

        // gittigi sitedaki tüm hayvanları öldürür.
        for (int i = 0; i < site->nanimals; i++) {
            Animal *animal = site->animals[i];
            animal->status = DEAD;
            remove_animal(site, animal);
        }
        usleep(1000);  // 1 milisaniye bekler
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    time_t startTime = time(NULL); // programın 1 saniye çalışması için başlangıç zamanı belirlenir.

    if (argc != 2) {
        printf("BÖyle kullanmalisin: ./main <hunter sayisi>\n");
        exit(1);
    }

    int n_hunters = atoi(argv[1]);
    if (n_hunters <= 0) {
        printf("Hunter sayisi pozitif olmali.\n");
        exit(1);
    }

    initgrid(5, 5);
    //griddeki animals ve hunters arraylari için yer ayrilir
    for(int i = 0;i<grid.xlength;i++){
        for(int i2 = 0;i2<grid.ylength;i2++){
            grid.sites[i][i2].animals = realloc(grid.sites[i][i2].animals, sizeof(Animal) * 100000);
            grid.sites[i][i2].hunters = realloc(grid.sites[i][i2].hunters, sizeof(Hunter) * 100000);
        }
    }
    
    //threaedler için array oluşturulur
    pthread_t animal_threads[3];
    pthread_t hunter_threads[n_hunters];

    //hayvanlar oluşturulur
    Animal bear = { ALIVE, BEAR, {0, 0} };
    Animal bird = { ALIVE, BIRD, {1, 1} };
    Animal panda = { ALIVE, PANDA, {2, 2} };
    pthread_create(&animal_threads[0], NULL, simulateanimal, &bear);
    pthread_create(&animal_threads[1], NULL, simulateanimal, &bird);
    pthread_create(&animal_threads[2], NULL, simulateanimal, &panda);
    
    // verilen hunter sayısı kadar hunter oluşturulur
    for (int i = 0; i < n_hunters; i++) {
        Hunter* hunter = malloc(sizeof(Hunter));
        hunter->points = 0;
        int x = rand() % grid.xlength;
        int y = rand() % grid.ylength;
        hunter->location = (Location) { x, y };
        pthread_mutex_lock(&m2);
        grid.sites[x][y].hunters[grid.sites[x][y].nhunters++] = hunter;
        pthread_mutex_unlock(&m2);
        pthread_create(&hunter_threads[i], NULL, simulatehunter, hunter);
    }
    
    // simulasyon zamanı 1 saniye olana kadar 1 milisaniye uyur.
    while (time(NULL) - startTime < 1) {
        usleep(1000);
    }

    // simulasyon bitmiştir grid ekrana yazdırılır.free işlemi yapılır son olarak simulasyonun çalışma suresi yazdırılır
    time_t endTime = time(NULL);
    printgrid();
    deletegrid();
    printf("simulasyonun çalişma süresi : %ld sn\n",endTime- startTime);

    return 0;
}