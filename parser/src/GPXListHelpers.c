#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "GPXParser.h"
#include "GPXHelper.h"
#include "LinkedListAPI.h"

void deleteGpxData( void* data){
    if(data == NULL){
        return;
    }

    GPXData *other = (GPXData *)data;

    free(other);
    return;
}

char* gpxDataToString( void* data){
    if(data == NULL){
        return NULL;
    }

    GPXData *other = (GPXData *) data;

    int nameLen = strlen(other->name);
    int valLen = strlen(other->value);

    char *str = malloc(sizeof(char) * (nameLen + valLen) + 100);
    sprintf(str, "Other data:\nname: %s, value: %s",other->name, other->value );

    return str;
}

int compareGpxData(const void *first, const void *second){

     if(first == NULL || second == NULL){
        return -1;
    }

    GPXData * other1 = (GPXData*)first;
    GPXData * other2 = (GPXData*)second;

    if( (strcmp(other1->value, other2->value)== 0) && (strcmp(other1->name, other2->name)== 0) ) {
        return 0;
    }else{
        return -1;
    }

}


/**********************************************************/
void deleteWaypoint(void* data){
    if(data == NULL){
        return;
    }
    
    Waypoint* wpt = (Waypoint*)data;
    
    free(wpt->name);
    freeList( wpt->otherData);
    free(wpt);
    return;
}

char* waypointToString( void* data){
    if(data == NULL){
        return NULL;
    }
    Waypoint* wpt = (Waypoint*)data;
    int nameLen = strlen(wpt->name);
    
    char* otherDataStr = toString(wpt->otherData); 
    int otherStrLen = strlen(otherDataStr);

    char *str = malloc(sizeof(char) * (nameLen + 100 + otherStrLen));

    sprintf(str, "%s %f %f%s\n", wpt->name, wpt->latitude, wpt->longitude, otherDataStr);
    
    free(otherDataStr);
    return str;
}


int compareWaypoints(const void *first, const void *second){

    if(first == NULL || second == NULL){
        return -1;
    }
    
    Waypoint * wpt1 = (Waypoint*)first;
    Waypoint * wpt2 = (Waypoint*)second;
    
    char* str1 = waypointToString(wpt1);
    char* str2 = waypointToString(wpt2);

    int x = strcmp(str1, str2);

    free(str1);
    free(str2);

    return x;
}



/**********************************************************/

void deleteRoute(void* data){
     if(data == NULL){
        return;
    }
    
    Route *rte = (Route*)data;

    free(rte->name);
    
    freeList(rte->otherData);
    
    freeList(rte->waypoints);
    free(rte);
    return;
}

char* routeToString(void* data){
    if(data == NULL){
        return NULL;
    }
    Route* rte = (Route*)data;
   

    char* rteOther = toString(rte->otherData);
    char* rteWpt = toString(rte->waypoints);

    char* str = malloc(sizeof(char)* (strlen(rte->name) +100 + strlen(rteOther) + strlen(rteWpt))); 

    sprintf(str, "Route Name: %s\n%s\n%s\n", rte->name, rteOther, rteWpt);

    free(rteOther);
    free(rteWpt);
    return str;
}

int compareRoutes(const void *first, const void *second){

    if(first == NULL || second == NULL){
        return -1;
    }

    Route * rte1 = (Route*)first;
    Route * rte2 = (Route*)second;

    char* str1 = routeToString(rte1);
    char* str2 = routeToString(rte2);

    int x = strcmp(str1, str2);

    free(str1);
    free(str2);

    return x;
}

/**********************************************************/

void deleteTrackSegment(void* data){
    if(data == NULL){
        return;
    }
    TrackSegment* trkseg = (TrackSegment*)data;

    freeList(trkseg->waypoints);
    free(trkseg);
    return;
}

char* trackSegmentToString(void* data){
    if(data == NULL){
        return NULL;
    }

    TrackSegment* trkseg = (TrackSegment*)data;
    char *wptStr = toString(trkseg->waypoints);

    char* str = malloc(sizeof(char)* strlen(wptStr)+ 100); 
    sprintf(str, "Track segment: %s", wptStr);

    free(wptStr);
    
    return str;  
}



int compareTrackSegments(const void *first, const void *second){
    if(first == NULL || second == NULL){
        return -1;
    }

    TrackSegment* trkseg1 = (TrackSegment*)first;
    TrackSegment* trkseg2 = (TrackSegment*)second;

    char* str1 = trackSegmentToString(trkseg1);
    char* str2 = trackSegmentToString(trkseg2);

    int x = strcmp(str1, str2);

    free(str1);
    free(str2);

    return x;
}

/**********************************************************/

void deleteTrack(void* data){

    if(data == NULL){   
        return;
    }
    Track* trk = (Track*)data;

    free(trk->name);
    freeList(trk->segments);
    freeList(trk->otherData);
    free(trk);

    return;
}


char* trackToString(void* data){

    if(data == NULL){   
        return NULL;
    }

    Track* trk = (Track*)data;

    char* other = toString(trk->otherData);
    char* trkseg = toString(trk->segments);

    char* str = malloc(sizeof(char)* (strlen(trk->name) +100 + strlen(trkseg) + strlen(other))); 

    sprintf(str, "Track Name: %s\n%s\n%s\n", trk->name, other, trkseg);

    free(other);
    free(trkseg);
    return str; 
}



int compareTracks(const void *first, const void *second){

    if(first == NULL || second == NULL){
        return -1;
    }
    Track* trk1 = (Track*)first;
    Track* trk2 = (Track*)second;

    char* str1 = trackToString(trk1);
    char* str2 = trackToString(trk2);

    int x = strcmp(str1, str2);

    free(str1);
    free(str2);

    return x;
}
