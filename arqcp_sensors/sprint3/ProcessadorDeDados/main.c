#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    int* write;
    int* read;
    int  size;
    int *arr;
} CircularBuffer;

// Definição da estrutura para um sensor
typedef struct {
    int sensor_id;
    char type[50];
    char unit[20];
    CircularBuffer* buffer;
    int* mediana;
    int elementosMediana;
    int last_read;
    int timeout;
    int write_counter;
    time_t last_received_time;
    int isInError;
} Sensor;

Sensor* setupSensor() {
    Sensor* mySensor = (Sensor*)malloc(sizeof(Sensor)); // Aloca a estrutura Sensor na heap
    mySensor->buffer = (CircularBuffer*)malloc(sizeof(CircularBuffer));
    mySensor->mediana = (int*)malloc(10 * sizeof(int)); // Alocando espaço para o array mediana

    return mySensor; // Retorna o ponteiro para a estrutura alocada dinamicamente na heap
}

Sensor initializeSensor(char* line) {
    Sensor sensor;
    char* token = strtok(line, "#");

    // Extract the data from the configuration file
    sensor.sensor_id = atoi(token);
    token = strtok(NULL, "#");

    strcpy(sensor.type, token);
    token = strtok(NULL, "#");
    strcpy(sensor.unit, token);
    token = strtok(NULL, "#");

    // Initialize CircularBuffer and set size
    int bufferSize = atoi(token);
    sensor.buffer = (CircularBuffer*)malloc(sizeof(CircularBuffer));
    sensor.buffer->arr = (int*)malloc(bufferSize * sizeof(int));
    sensor.buffer->read = (int*)malloc(sizeof(int));
	sensor.buffer->write = (int*)malloc(sizeof(int));
	*(sensor.buffer->read) = 0;
	*(sensor.buffer->write) = 0;
    sensor.buffer->size = bufferSize;

    // Set window_len, timeout, write counter, and initialize median array
    token = strtok(NULL, "#");
    int windowLen = atoi(token);
    int comprimento_mediana = bufferSize - windowLen + 1;
    sensor.timeout = atoi(strtok(NULL, "#"));
    sensor.write_counter = 0;
	sensor.isInError = 0;
    sensor.mediana = (int*)malloc(comprimento_mediana * sizeof(int)); 
	//sensor.valorMediana = 0;
	sensor.elementosMediana = comprimento_mediana;		
    // Set last read to timeout initially
    sensor.last_read =0;

    return sensor;
}

void receberInfoSensor(Sensor **arraySensor, int numSensores) {
    const char *nomeDoArquivo = "infoSensores.txt";
    const int intervaloSegundos = 10;  
    char serialize[256];
    char * ptrSerialize = serialize;    
    int mediana = 0;
    while (1) {
        int contadorLeitura = 0;
        int d = 10;
        FILE *arquivo = fopen(nomeDoArquivo, "r");
  
        if (arquivo != NULL) {
            char linha[256];
          
		while (fgets(linha, sizeof(linha), arquivo) != NULL && contadorLeitura < d) {                
                int n = 0;    
				int * output = &n;
				extract_token(linha,"sensor_id:",output);
				Sensor* sensor = arraySensor[(*output) - 1];
                insertSensorData(linha, sensor);

                // Atualiza o tempo da última leitura
                time(&(sensor->last_received_time));

                // Verifica se o sensor ultrapassou o timeout
                if (difftime(time(NULL), sensor->last_received_time) > sensor->timeout) {
                    printf("Alerta: Sensor %d ultrapassou o timeout!\n", sensor->sensor_id);
					// poe o parametro que indica se esta em erro ou nao a 1
					sensor->isInError = 1;
				}
			    contadorLeitura++;
		}
                for (int i = 0; i < numSensores; i++) {
               
				calculateMovingMedian(arraySensor[i]);
                printf("aaa");
                serializeSensorInfo(arraySensor[i], 10, numSensores,256);
				
				}		
             
        
        } else {
            fprintf(stderr, "Erro ao abrir o arquivo.\n");
        }
		break;
        //sleep(intervaloSegundos);
    }

    return 0;
}

void serializeSensorInfo(Sensor* sensor, char* arraySerialize,int mediana,int tamanhoArraySerialize) {
    for (int i = 0; i < tamanhoArraySerialize; i++) {
        // Adiciona informações ao arraySerialize no formato sensor_id,write_counter,type,unit,mediana#
        sprintf(arraySerialize, "%d,%d,%s,%s,%d#", 
                sensor[i].sensor_id, 
                sensor[i].write_counter, 
                sensor[i].type, 
                sensor[i].unit
           );

        
        // Move o ponteiro para o próximo local no arraySerialize
        arraySerialize += strlen(arraySerialize);
    
    }

}


void insertSensorData(char* sensorData,Sensor * sensor) {    
    int n = 0;    
    int * output = &n;
    extract_token(sensorData,"value:",output);
    enqueue_value(sensor->buffer->arr, sensor->buffer->size, sensor->buffer->read, sensor->buffer->write, *output);	
}

void printCircularBuffer(const CircularBuffer* buffer) {
    printf("Circular Buffer Content for buffer-:\n");

    int readIndex = *(buffer->read);
    int writeIndex = *(buffer->write);

    if (readIndex == writeIndex) {
        printf("Buffer is empty\n");
        return;
    }

    for (int i = 0; i < buffer->size; i++) {
        int index = (readIndex + i) % buffer->size;
        printf("%d ", buffer->arr[index]);
    }

    printf("\n");
}

void writeCircularBufferToFile(FILE* arquivo, const CircularBuffer* buffer) {
    fprintf(arquivo, "Circular Buffer Content for buffer-> ");
    int readIndex = *(buffer->read);
    int writeIndex = *(buffer->write);

    if (readIndex == writeIndex) {
        fprintf(arquivo, "Buffer is empty\n");
        return;
    }

    for (int i = 0; i < buffer->size; i++) {
        int index = (readIndex + i) % buffer->size;
        fprintf(arquivo, "%d ", buffer->arr[index]);
    }
	fprintf(arquivo, "\n");
	fprintf(arquivo, "Buffer size: %d\n", buffer->size);
}


void copyElements(int* source, int* destination, int x) {
    memcpy(destination, source, x * sizeof(int));
    sort_array(destination,x);
}



void calculateMovingMedian(Sensor* sensor) {
    CircularBuffer* buffer = sensor->buffer;
    int* bufferArray = buffer->arr;
    int bufferSize = buffer->size;
    int* medianaArray = sensor->mediana;
	int windowSize = sensor->elementosMediana;
    int r = bufferSize + 1 - windowSize;
    int array3 [r];
    printf("SENSOR %d\n", sensor->sensor_id);

	for (int i = 0; i < windowSize; i++) {
		
		int index = (*(buffer->read) + i) % buffer->size;
		
    if (index < 0) {
        index += buffer->size;
    }

    int value = buffer->arr[index];

    copyElements(buffer->arr + index, array3 , r);

    medianaArray[i] = mediana(array3,r);    
	printf("windowSIze %d\n", windowSize);
	printf("I %d\n", i);
	}
	
	//sort_array(medianaArray, windowSize);
//	printf("MEDIANA %d MAMAMA %d",medianaArray,windowSize);
//	sensor->valorMediana = mediana(medianaArray, windowSize);
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Uso: %s <caminho_coletor> <arquivo_config> <diretorio_saida> <num_leituras>\n", argv[0]);
        return 1;
    }

    char *caminho_coletor = argv[1];
    char *arquivo_config = argv[2];
    char *diretorio_saida = argv[3];
    int num_leituras = atoi(argv[4]); // Convertendo o último argumento para inteiro


    FILE* configFile = fopen(arquivo_config, "r");
    if (configFile == NULL) {
        printf("Erro ao abrir o arquivo de configuração.\n");
        return 1;
    }

    Sensor** sensorArray = (Sensor**)malloc(num_leituras * sizeof(Sensor*));
    if (sensorArray == NULL) {
        printf("Erro ao alocar memória para o array de sensores.\n");
        fclose(configFile);
        return 1;
    }

    char line[100];
    int sensorIndex = 0;
    while (fgets(line, sizeof(line), configFile) != NULL && sensorIndex < num_leituras) {
        Sensor* sensorPtr = setupSensor();
        Sensor sensor = initializeSensor(line);
        *sensorPtr = sensor;
        sensorArray[sensorIndex] = sensorPtr;
       
        sensorIndex++;
    }
	
    fclose(configFile);
	
	receberInfoSensor(sensorArray, num_leituras);
	
	printf ("\n");
	for (int i = 0; i < sensorIndex; i++) {
		Sensor* currentSensor = sensorArray[i];
		printf("Sensor ID: %d, Type: %s, Unit: %s\n", currentSensor->sensor_id, currentSensor->type, currentSensor->unit);
		printf("Buffer Size: %d, Last Read: %d, Timeout: %d, Write Counter: %d\n",
			   currentSensor->buffer->size, currentSensor->last_read, currentSensor->timeout, currentSensor->write_counter);
		printf("Median Array Size: %d\n", currentSensor->buffer->size); // Print median array size

		// Access other sensor data as needed
	}

    
    
    
    for (int i = 0; i < sensorIndex; i++) {
    // Free CircularBuffer data array
    free(sensorArray[i]->buffer->arr);

    // Free CircularBuffer struct
    free(sensorArray[i]->buffer);
    free(sensorArray[i]->mediana);
    // Free Sensor struct
    free(sensorArray[i]);
}

// Free sensorArray itself
free(sensorArray);

    return 0;
}
