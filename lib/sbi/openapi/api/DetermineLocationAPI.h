#include <stdlib.h>
#include <stdio.h>
#include "../include/apiClient.h"
#include "../include/list.h"
#include "../external/cJSON.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"
#include "../model/input_data.h"
#include "../model/location_data.h"
#include "../model/problem_details.h"
#include "../model/redirect_response.h"


// Determine Location of an UE
//
location_data_t*
DetermineLocationAPI_determineLocation(apiClient_t *apiClient, input_data_t * input_data );


