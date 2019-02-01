
//Per far funzionare il codice, è sufficiente digitare make dal terminale che punta sulla cartella.
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

#include<math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#define MAX_SOURCE_SIZE (0x100000)



int main(int argc, char ** argv) {
	int n=3;//PER FARE I TEST, CAMBIARE QUESTO VALORE.
	int SIZE = n*n;//ALLOCO GLI ARRAYVISTI COME MATRICI

	
	int *A = (int*)malloc(sizeof(int)*SIZE);
	int *B = (int*)malloc(sizeof(int)*SIZE);

	
	int *C = (int*)malloc(sizeof(int)*SIZE);
	
	
	//INIZIALIZZO I VALORI DELLE MATRICI
	int i = 0;
	for (i=0; i<SIZE; ++i) {
		A[i] = i+1;//INIZIALIZZO LE MATRICI COME QUELLE SEQUENZIALI
		B[i] = (i+1)*2;
		C[i]=0;
	}
	
	printf("MATRICE A PARALLELA\n\n");

	for(i=0;i<SIZE;i++){
	printf(" %d ",A[i]);
	if(((i+1)%n)==0){printf("\n");}
	}

	printf("MATRICE B PARALLELA\n\n");

	for(i=0;i<SIZE;i++){
	printf(" %d ",B[i]);
	if(((i+1)%n)==0){printf("\n");}
	}

	printf("MATRICE C PARALLELA\n\n");

	for(i=0;i<SIZE;i++){
	printf(" %d ",C[i]);
	if(((i+1)%n)==0){printf("\n");}
	}




	//INIZIO AD ALLOCARE LE MATRICI PER LA MOLTIPLICAZIONE SEQUENZIALE

	int (*a1)[n];
	a1=(int (*)[n])malloc(sizeof(*a1)*n);

	int (*b1)[n];
	b1=(int (*)[n])malloc(sizeof(*b1)*n);

	int (*c1)[n];
	c1=(int (*)[n])malloc(sizeof(*c1)*n);

	
	int tmp=1;
	for(int i=0;i<n;i++){//INIZIALIZZO LE MATRICI SEQUENZIALI NELLO STESSO MODO DELLE PRIME
		for(int j=0;j<n;j++){
		a1[i][j]=tmp;
		b1[i][j]=(tmp)*2;
		c1[i][j]=0;
		tmp+=1;
			
		}
		
	}//LE STAMPO PER SICUREZZA ANCHE LE MATRICI SEQUENZIALI

	printf("MATRICE A SEQUENZIALE\n\n");

	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
		printf(" %d ",a1[i][j]);
		}
		printf("\n");
	}

	printf("MATRICE B SEQUENZIALE\n\n");

	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
		printf(" %d ",b1[i][j]);
		}
		printf("\n");
	}

	printf("MATRICE C SEQUENZIALE\n\n");

	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
		printf(" %d ",c1[i][j]);
		}
		printf("\n");
	}

	//INIZIO FACENDO LA MOLTIPLICAZIONE SEQUENZIALE

	clock_t start,end;
	double tempo;//DICHIARO UNA VARIABILE PER IL TEMPO
	start=clock();//PRENDO IL TEMPO PRIMA DELL'ESECUZIONE


	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			for(int k=0;k<n;k++){
				c1[i][j]+=a1[i][k]*b1[k][j];			
			}		
		}
	}


	end=clock();//PRENDO IL TEMPO DOPO L'ESECUZIONE
	tempo=((double)(end-start))/CLOCKS_PER_SEC;
	//ESEGUO LA SOTTRAZIONE PER VEDERE QUANTO TEMPO HA IMPIEGATO LA MOLTIPLICAZIONE


	printf("MATRICE C DOPO MOLTIPLICAZIONE SEQUENZIALE\n\n");
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
		printf(" %d ",c1[i][j]);
		}
		printf("\n");
	}

	

	FILE *kernelFile;//VARIABILI CHE CONTERRANNO IL KERNEL
	char *kernelSource;
	size_t kernelSize;

	kernelFile = fopen("kernel.cl", "r");//APRO IL FILE CON IL KERNEL

	if (!kernelFile) {

		fprintf(stderr, "Non ho trovato nessun file chiamato kernel.cl\n");

		exit(-1);//termino il programma se non trovo il kernel

	}
	kernelSource = (char*)malloc(MAX_SOURCE_SIZE);
	kernelSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, kernelFile);
	fclose(kernelFile);
	//una volta definite le variabili con il kernel letto, chiudo il file
	// Prendo informazioni sulle piattaforme e i devices
	cl_platform_id platformId = NULL;
	cl_device_id deviceID = NULL;
	cl_uint retNumDevices;
	cl_uint retNumPlatforms;
	cl_int ret = clGetPlatformIDs(1, &platformId, &retNumPlatforms);
	ret = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_DEFAULT, 1, &deviceID, &retNumDevices);

	// Creo il contesto
	cl_context context = clCreateContext(NULL, 1, &deviceID, NULL, NULL,  &ret);


	// Creo la coda dei comandi
	cl_command_queue commandQueue = clCreateCommandQueue(context, deviceID, 0, &ret);

	// creo i buffer per ogni array
	cl_mem aMemObj = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * sizeof(int), NULL, &ret);
	cl_mem bMemObj = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * sizeof(int), NULL, &ret);
	cl_mem cMemObj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SIZE * sizeof(int), NULL, &ret);


	// copio le liste ai buffer definiti in precedenza
	ret = clEnqueueWriteBuffer(commandQueue, aMemObj, CL_TRUE, 0, SIZE * sizeof(int), A, 0, NULL, NULL);;
	ret = clEnqueueWriteBuffer(commandQueue, bMemObj, CL_TRUE, 0, SIZE * sizeof(int), B, 0, NULL, NULL);

	// creo il programma dal codice del kernel
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, (const size_t *)&kernelSize, &ret);	

	// compilo il programma
	ret = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL);

	// Creo l'oggetto kernel
	cl_kernel kernel = clCreateKernel(program, "matrice_mul", &ret);


	// Setto gli argomenti del kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&aMemObj);	
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bMemObj);	
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&cMemObj);
	ret = clSetKernelArg(kernel, 3, sizeof(int),(void *)&n);		

	clock_t startb,endb;
	double tempob;//prendo il tempo 
	startb=clock();



	size_t localWorkSize[2], globalWorkSize[2];
 	localWorkSize[0] = 1;
   	localWorkSize[1] = 1;
   	globalWorkSize[0] = n;
   	globalWorkSize[1] = n;
	
	//chiamiamo l'esecuzione del kernel
	
	ret = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);	

	//prendiamo i risultati dal device all'host
	ret = clEnqueueReadBuffer(commandQueue, cMemObj, CL_TRUE, 0, SIZE * sizeof(int), C, 0, NULL, NULL);



	endb=clock();
	tempob=((double)(endb-startb))/CLOCKS_PER_SEC;//prendo il tempo alla fine dell'esecuzione

	printf("MATRICE C RISULTANTE DOPO MOLTIPLICAZIONE PARALLELA\n\n");

	for(i=0;i<SIZE;i++){
		printf(" %d ",C[i]);
		if(((i+1)%n)==0){printf("\n");}
	}

	//FACCIO IL CONTROLLO SUL RISULTATO PER VEDERE SE È UGUALE c1 a C

	bool temp=false;
	int tmpa=0;
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			if(C[tmpa]!=c1[i][j]){
				temp=true;//mi basta un elemento diverso per lanciare un errore
				break;		
			}else{
			tmpa=tmpa+1;		
			}
		}
	}

	if(temp){
	printf("Qualcosa è andato storto, le matrici risultanti non sono uguali, ricontrolla il codice\n");

		
	}else{
	printf("\n\nMoltiplicazioni uguali avvenute con successo\n\n");
	printf("TEMPO MOLTIPLICAZIONE SEQUENZIALE = %f\n",tempo);	
	printf("TEMPO MOLTIPLICAZIONE PARALLELA = %f\n\n",tempob);	
	}

	// rilascio la memoria
	ret = clFlush(commandQueue);
	ret = clFinish(commandQueue);
	ret = clReleaseCommandQueue(commandQueue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(aMemObj);
	ret = clReleaseMemObject(bMemObj);
	ret = clReleaseMemObject(cMemObj);
	ret = clReleaseContext(context);
	free(A);
	free(B);
	free(C);

	return 0;

}
