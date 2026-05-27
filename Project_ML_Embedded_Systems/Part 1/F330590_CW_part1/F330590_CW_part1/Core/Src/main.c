/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "stm32f7xx_ll_cortex.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct linked_list linked_list_t;
typedef struct neural_net neural_net_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim11;
UART_HandleTypeDef huart3;
PCD_HandleTypeDef hpcd_USB_OTG_FS;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_TIM11_Init(void);
/* USER CODE BEGIN PFP */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/**
* @brief Retargets the C library printf function to the USART.
* @param None
* @retval None
*/
PUTCHAR_PROTOTYPE
{
/* Place your implementation of fputc here */
/* e.g. write a character to the USART1 and Loop until the end of transmission */
	HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

linked_list_t* new_linked_list();

bool linked_list_append(linked_list_t *list, void *data);

void *linked_list_remove(linked_list_t *list, int index);

bool linked_list_start_iterator(linked_list_t *list);

void *linked_list_get_next(linked_list_t *list);

int linked_list_length(linked_list_t *list);

void *linked_list_get_last(linked_list_t *list);

neural_net_t* create_neural_net();

void neural_net_add_layer(neural_net_t *neural_net);

void neural_net_add_neuron(neural_net_t *neural_net, double *weights, int n_weights, double bias);

double * neural_net_run(neural_net_t* neural_net, double *data, int len);

uint32_t getCurrentMicros(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char uart_buf[50];
int uart_buf_len;
uint16_t timer_val;

int classify (double *results, int size_results){

    int final_class = 0;
    double final_score = *results;
    int i;

    for(i = 1 ; i < size_results ; i++){

        if(results[i] > final_score){
            final_class = i;
            final_score = results[i];

        }

    }

    return final_class;
}

uint32_t getCurrentMicros(void)
{
	LL_SYSTICK_IsActiveCounterFlag();
	uint32_t m = HAL_GetTick();
	const uint32_t tms = SysTick->LOAD + 1;
	__IO uint32_t u = tms - SysTick->VAL;
	if (LL_SYSTICK_IsActiveCounterFlag())
	{
		m = HAL_GetTick();
		u = tms - SysTick->VAL;
	}
	return (m*1000+(u*1000)/tms);
}


struct neural_net{

	    linked_list_t *layers;

	};

	typedef struct{

	    linked_list_t* neurons;
	//  double bias;


	}layer_t;


	typedef struct{

	    double* weights;
	    double bias;
	    int n_weights;


	}neuron_t;

	neural_net_t* create_neural_net(){

	    neural_net_t *neural_net = (neural_net_t*)malloc(sizeof(neural_net_t));
	    neural_net->layers = new_linked_list();

	    return neural_net;

	}

	void neural_net_add_layer(neural_net_t *neural_net){

	    layer_t *layer = malloc(sizeof(layer_t));
	    layer->neurons = new_linked_list();
	    linked_list_append(neural_net->layers, layer);

	}

	void neural_net_add_neuron(neural_net_t *neural_net, double *weights, int n_weights, double bias){

	    layer_t *last_layer = linked_list_get_last(neural_net->layers);

	    neuron_t *neuron  = (neuron_t*)malloc(sizeof(neuron_t));
	    neuron->weights = weights;
	    neuron->n_weights = n_weights;
	    neuron->bias = bias;

	    linked_list_append(last_layer->neurons, neuron);

	}


	double _activation_func(double val){
	    return 1/(1+exp(-val));
	}

	double _neuron_evaluate(neuron_t* neuron, double *data){

	    int i;
	    double result;
	    for(i = 0, result = 0 ; i <  neuron->n_weights ; i++){

	        result += neuron->weights[i] * data[i];

	    }


	    return _activation_func(result + neuron->bias);
	}


	double * neural_net_run(neural_net_t* neural_net, double *data, int len){

	    layer_t *layer;
	    neuron_t *neuron;
	    int i;
	    double result;
	    double *prev_outputs = (double *)malloc(sizeof(double)*len);
	    double *next_outputs = NULL;

	    linked_list_start_iterator(neural_net->layers);


	    memcpy(prev_outputs, data, sizeof(double)*len);

	    while(layer = linked_list_get_next(neural_net->layers)){

	        linked_list_start_iterator(layer->neurons);
	        result = 0;
	        next_outputs = malloc(sizeof(double) * (linked_list_length(layer->neurons)));
	//      next_outputs[linked_list_length(layer->neurons)] = layer->bias;
	        i = 0;

	        while(neuron = linked_list_get_next(layer->neurons)){

	            next_outputs[i] = _neuron_evaluate(neuron, prev_outputs);
	            i++;

	        }

	        free(prev_outputs);
	        prev_outputs = next_outputs;


	    }

	    return prev_outputs;
	}



	typedef struct node node_t;

	struct node{

	    void *data;
	    node_t *next;

	};


	struct linked_list{

	    node_t *head;
	    node_t *tail;
	    node_t *iterator;
	    int length;

	};


	linked_list_t* new_linked_list(){

	    linked_list_t *linked_list = (linked_list_t*)malloc(sizeof(linked_list_t));

	    linked_list->head = NULL;
	    linked_list->iterator = NULL;
	    linked_list->tail = NULL;
	    linked_list->length = 0;

	    return linked_list;

	}

	bool linked_list_append(linked_list_t *list, void *data){

	    node_t *node = (node_t*) malloc(sizeof(node_t));

	    node->data = data;
	    node->next = NULL;

	    if(list->head == NULL){
	        list->head = node;
	        list->tail = node;
	    }else{
	        list->tail->next = node;
	        list->tail = node;
	    }

	    list->length++;

	}

	void *linked_list_remove(linked_list_t *list, int index){


	    if(index >= list->length){
	        return NULL;
	    }

	    if(index == 0){

	        void *data = list->head->data;
	        node_t *node_to_remove = list->head;
	        list->head = list->head->next;

	        free(node_to_remove);

	        if(list->head == NULL){
	            list->tail == NULL;
	        }

	        list->length--;
	        return data;

	    }


	    int current_index = 0;
	    node_t* node = list->head;

	    while(current_index < index - 1){
	        current_index++;
	        node = node->next;
	    }

	    node_t* node_to_remove = node->next;
	    node->next = node_to_remove->next;
	    void *data = node_to_remove->data;

	    if(index == list->length - 1){
	        list->tail = node;
	    }

	    free(node_to_remove);
	    list->length--;

	    return data;


	}

	bool linked_list_start_iterator(linked_list_t *list){
	    list->iterator = list->head;
	}

	void *linked_list_get_next(linked_list_t *list){

	    if(list->iterator == NULL){
	        return NULL;
	    }

	    void *data = list->iterator->data;
	    list->iterator = list->iterator->next;
	    return data;

	}

	int linked_list_length(linked_list_t *list){

	    return list->length;

	}


	void *linked_list_get_last(linked_list_t *list){

	    return list->tail->data;

	}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim11);
  int i;
  const double *result;
  int run;
  int max_run = 10; // Change this value to change the number of runs.
  uint32_t run_times[max_run];
  uint32_t start_time;
  uint32_t end_time;
  uint32_t run_time;
  uint32_t total_time = 0;
  uint32_t avg_time = 0;

      neural_net_t *neural_net = create_neural_net();


      neural_net_add_layer(neural_net);
      double w_0_0[] = { 0.18284023, -0.8529724 ,  0.9295352 ,  0.27745247};
      neural_net_add_neuron(neural_net, w_0_0,  4, -0.5623624);

      double w_0_1[] = {-0.17880775, -0.42715448,  0.5908963 ,  0.45988828};
      neural_net_add_neuron(neural_net, w_0_1,  4, -0.05714933);

      double w_0_2[] = {-0.48771185, -0.3112061 , -0.3137951 ,  0.5289104 };
      neural_net_add_neuron(neural_net, w_0_2,  4, -0.053803623);

      double w_0_3[] = { 0.24778503,  0.9340235 , -1.4063437 , -1.3005494 };
      neural_net_add_neuron(neural_net, w_0_3,  4, 0.27202815);

      double w_0_4[] = { 0.72132677, -0.12656425, -1.1483048 , -0.083784  };
      neural_net_add_neuron(neural_net, w_0_4,  4, 0.41053805);

      double w_0_5[] = { 0.05532144,  0.47561175, -0.17761162, -1.3145427 };
      neural_net_add_neuron(neural_net, w_0_5,  4, 0.28362706);

      double w_0_6[] = { 0.12644108, -0.6311569 ,  0.03945397, -0.53849036};
      neural_net_add_neuron(neural_net, w_0_6,  4, 0.025350625);

      double w_0_7[] = { 0.21844819,  0.30629316, -0.961491  , -0.46816182};
      neural_net_add_neuron(neural_net, w_0_7,  4, 0.6091357);

      neural_net_add_layer(neural_net);
      double w_1_0[] = {-1.2635437 , -1.316526  ,  0.28543112,  1.3884338 ,  0.45282924,
        1.0371606 ,  0.12748991,  0.6325952 };
      neural_net_add_neuron(neural_net, w_1_0,  8, -0.43309692);

      double w_1_1[] = { 0.497385  , -0.632777  ,  0.38966534, -1.2256211 , -0.19634834,
        0.25377455,  0.6375568 , -0.5464157 };
      neural_net_add_neuron(neural_net, w_1_1,  8, -0.14760242);

      double w_1_2[] = { 0.3835234 ,  0.10146759, -0.32170305, -1.1918751 , -1.1514843 ,
       -0.8062826 , -0.69583875, -0.74578637};
      neural_net_add_neuron(neural_net, w_1_2,  8, 0.14179966);


      	static const double test_data[150][6] = {
        {0,5.1,3.5,1.4,0.2,0.0},
        {1,4.9,3.0,1.4,0.2,0.0},
        {2,4.7,3.2,1.3,0.2,0.0},
        {3,4.6,3.1,1.5,0.2,0.0},
        {4,5.0,3.6,1.4,0.2,0.0},
        {5,5.4,3.9,1.7,0.4,0.0},
        {6,4.6,3.4,1.4,0.3,0.0},
        {7,5.0,3.4,1.5,0.2,0.0},
        {8,4.4,2.9,1.4,0.2,0.0},
        {9,4.9,3.1,1.5,0.1,0.0},
        {10,5.4,3.7,1.5,0.2,0.0},
        {11,4.8,3.4,1.6,0.2,0.0},
        {12,4.8,3.0,1.4,0.1,0.0},
        {13,4.3,3.0,1.1,0.1,0.0},
        {14,5.8,4.0,1.2,0.2,0.0},
        {15,5.7,4.4,1.5,0.4,0.0},
        {16,5.4,3.9,1.3,0.4,0.0},
        {17,5.1,3.5,1.4,0.3,0.0},
        {18,5.7,3.8,1.7,0.3,0.0},
        {19,5.1,3.8,1.5,0.3,0.0},
        {20,5.4,3.4,1.7,0.2,0.0},
        {21,5.1,3.7,1.5,0.4,0.0},
        {22,4.6,3.6,1.0,0.2,0.0},
        {23,5.1,3.3,1.7,0.5,0.0},
        {24,4.8,3.4,1.9,0.2,0.0},
        {25,5.0,3.0,1.6,0.2,0.0},
        {26,5.0,3.4,1.6,0.4,0.0},
        {27,5.2,3.5,1.5,0.2,0.0},
        {28,5.2,3.4,1.4,0.2,0.0},
        {29,4.7,3.2,1.6,0.2,0.0},
        {30,4.8,3.1,1.6,0.2,0.0},
        {31,5.4,3.4,1.5,0.4,0.0},
        {32,5.2,4.1,1.5,0.1,0.0},
        {33,5.5,4.2,1.4,0.2,0.0},
        {34,4.9,3.1,1.5,0.2,0.0},
        {35,5.0,3.2,1.2,0.2,0.0},
        {36,5.5,3.5,1.3,0.2,0.0},
        {37,4.9,3.6,1.4,0.1,0.0},
        {38,4.4,3.0,1.3,0.2,0.0},
        {39,5.1,3.4,1.5,0.2,0.0},
        {40,5.0,3.5,1.3,0.3,0.0},
        {41,4.5,2.3,1.3,0.3,0.0},
        {42,4.4,3.2,1.3,0.2,0.0},
        {43,5.0,3.5,1.6,0.6,0.0},
        {44,5.1,3.8,1.9,0.4,0.0},
        {45,4.8,3.0,1.4,0.3,0.0},
        {46,5.1,3.8,1.6,0.2,0.0},
        {47,4.6,3.2,1.4,0.2,0.0},
        {48,5.3,3.7,1.5,0.2,0.0},
        {49,5.0,3.3,1.4,0.2,0.0},
        {50,7.0,3.2,4.7,1.4,1.0},
        {51,6.4,3.2,4.5,1.5,1.0},
        {52,6.9,3.1,4.9,1.5,1.0},
        {53,5.5,2.3,4.0,1.3,1.0},
        {54,6.5,2.8,4.6,1.5,1.0},
        {55,5.7,2.8,4.5,1.3,1.0},
        {56,6.3,3.3,4.7,1.6,1.0},
        {57,4.9,2.4,3.3,1.0,1.0},
        {58,6.6,2.9,4.6,1.3,1.0},
        {59,5.2,2.7,3.9,1.4,1.0},
        {60,5.0,2.0,3.5,1.0,1.0},
        {61,5.9,3.0,4.2,1.5,1.0},
        {62,6.0,2.2,4.0,1.0,1.0},
        {63,6.1,2.9,4.7,1.4,1.0},
        {64,5.6,2.9,3.6,1.3,1.0},
        {65,6.7,3.1,4.4,1.4,1.0},
        {66,5.6,3.0,4.5,1.5,1.0},
        {67,5.8,2.7,4.1,1.0,1.0},
        {68,6.2,2.2,4.5,1.5,1.0},
        {69,5.6,2.5,3.9,1.1,1.0},
        {70,5.9,3.2,4.8,1.8,1.0},
        {71,6.1,2.8,4.0,1.3,1.0},
        {72,6.3,2.5,4.9,1.5,1.0},
        {73,6.1,2.8,4.7,1.2,1.0},
        {74,6.4,2.9,4.3,1.3,1.0},
        {75,6.6,3.0,4.4,1.4,1.0},
        {76,6.8,2.8,4.8,1.4,1.0},
        {77,6.7,3.0,5.0,1.7,1.0},
        {78,6.0,2.9,4.5,1.5,1.0},
        {79,5.7,2.6,3.5,1.0,1.0},
        {80,5.5,2.4,3.8,1.1,1.0},
        {81,5.5,2.4,3.7,1.0,1.0},
        {82,5.8,2.7,3.9,1.2,1.0},
        {83,6.0,2.7,5.1,1.6,1.0},
        {84,5.4,3.0,4.5,1.5,1.0},
        {85,6.0,3.4,4.5,1.6,1.0},
        {86,6.7,3.1,4.7,1.5,1.0},
        {87,6.3,2.3,4.4,1.3,1.0},
        {88,5.6,3.0,4.1,1.3,1.0},
        {89,5.5,2.5,4.0,1.3,1.0},
        {90,5.5,2.6,4.4,1.2,1.0},
        {91,6.1,3.0,4.6,1.4,1.0},
        {92,5.8,2.6,4.0,1.2,1.0},
        {93,5.0,2.3,3.3,1.0,1.0},
        {94,5.6,2.7,4.2,1.3,1.0},
        {95,5.7,3.0,4.2,1.2,1.0},
        {96,5.7,2.9,4.2,1.3,1.0},
        {97,6.2,2.9,4.3,1.3,1.0},
        {98,5.1,2.5,3.0,1.1,1.0},
        {99,5.7,2.8,4.1,1.3,1.0},
        {100,6.3,3.3,6.0,2.5,2.0},
        {101,5.8,2.7,5.1,1.9,2.0},
        {102,7.1,3.0,5.9,2.1,2.0},
        {103,6.3,2.9,5.6,1.8,2.0},
        {104,6.5,3.0,5.8,2.2,2.0},
        {105,7.6,3.0,6.6,2.1,2.0},
        {106,4.9,2.5,4.5,1.7,2.0},
        {107,7.3,2.9,6.3,1.8,2.0},
        {108,6.7,2.5,5.8,1.8,2.0},
        {109,7.2,3.6,6.1,2.5,2.0},
        {110,6.5,3.2,5.1,2.0,2.0},
        {111,6.4,2.7,5.3,1.9,2.0},
        {112,6.8,3.0,5.5,2.1,2.0},
        {113,5.7,2.5,5.0,2.0,2.0},
        {114,5.8,2.8,5.1,2.4,2.0},
        {115,6.4,3.2,5.3,2.3,2.0},
        {116,6.5,3.0,5.5,1.8,2.0},
        {117,7.7,3.8,6.7,2.2,2.0},
        {118,7.7,2.6,6.9,2.3,2.0},
        {119,6.0,2.2,5.0,1.5,2.0},
        {120,6.9,3.2,5.7,2.3,2.0},
        {121,5.6,2.8,4.9,2.0,2.0},
        {122,7.7,2.8,6.7,2.0,2.0},
        {123,6.3,2.7,4.9,1.8,2.0},
        {124,6.7,3.3,5.7,2.1,2.0},
        {125,7.2,3.2,6.0,1.8,2.0},
        {126,6.2,2.8,4.8,1.8,2.0},
        {127,6.1,3.0,4.9,1.8,2.0},
        {128,6.4,2.8,5.6,2.1,2.0},
        {129,7.2,3.0,5.8,1.6,2.0},
        {130,7.4,2.8,6.1,1.9,2.0},
        {131,7.9,3.8,6.4,2.0,2.0},
        {132,6.4,2.8,5.6,2.2,2.0},
        {133,6.3,2.8,5.1,1.5,2.0},
        {134,6.1,2.6,5.6,1.4,2.0},
        {135,7.7,3.0,6.1,2.3,2.0},
        {136,6.3,3.4,5.6,2.4,2.0},
        {137,6.4,3.1,5.5,1.8,2.0},
        {138,6.0,3.0,4.8,1.8,2.0},
        {139,6.9,3.1,5.4,2.1,2.0},
        {140,6.7,3.1,5.6,2.4,2.0},
        {141,6.9,3.1,5.1,2.3,2.0},
        {142,5.8,2.7,5.1,1.9,2.0},
        {143,6.8,3.2,5.9,2.3,2.0},
        {144,6.7,3.3,5.7,2.5,2.0},
        {145,6.7,3.0,5.2,2.3,2.0},
        {146,6.3,2.5,5.0,1.9,2.0},
        {147,6.5,3.0,5.2,2.0,2.0},
        {148,6.2,3.4,5.4,2.3,2.0},
        {149,5.9,3.0,5.1,1.8,2.0}
    };

      for(run = 0; run < max_run; run++){
          run_time = 0;
          for(i = 0; i < 150; i++){
              start_time = getCurrentMicros();
              result = neural_net_run(neural_net, test_data[i] + 1, 4);
              end_time = getCurrentMicros();
              run_time += (end_time - start_time);

              printf("%lf %lf %lf -> %d\n", *result, result[1], result[2], classify(result, 3));
              free(result);
          }
          run_times[run] = run_time; // The current run_time is appended to the array run_times, so that it can be accessed later.
          total_time += run_time;
      }

      // run_times stored in the array are accessed one by one and printed at the end the execution time of each run.
      for(run = 0; run < max_run; run++){
          uart_buf_len = sprintf(uart_buf, "Run %d: %lu us\r", run + 1, run_times[run]);
          HAL_UART_Transmit(&huart3, (uint8_t *)uart_buf, uart_buf_len, 100);
      }

      // avg_time is calculated and then output alongside how many runs are conducted for the average.
      avg_time = total_time / max_run;
      uart_buf_len = sprintf(uart_buf, "Average inference time across %d runs: %lu us\r\n", max_run, avg_time);
      HAL_UART_Transmit(&huart3, (uint8_t *)uart_buf, uart_buf_len, 100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 96;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 65535;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */

/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_TXD1_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_TX_EN_Pin RMII_TXD0_Pin */
  GPIO_InitStruct.Pin = RMII_TX_EN_Pin|RMII_TXD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */

/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
