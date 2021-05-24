#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "GPXParser.h"
#include "GPXHelper.h"
#include "LinkedListAPI.h"
#include <stdbool.h>

bool findWaypoint(const void* a, const void* b){

    if(a == NULL || b == NULL){
        return false;
    }

    Waypoint* first = (Waypoint*)a;
    char* second = (char*)b;
    
    if(strcmp(first->name, second) == 0){
        return true;
    }else{
        return false;
    }
    
}


bool findRoute(const void* a, const void* b){

    if(a == NULL || b == NULL){
        return false;
    }
    
    Route* first = (Route*)a;
    char* second = (char*)b;
    
    if(strcmp(first->name, second) == 0){
        return true;
    }else{
        return false;
    }
    
}

bool findTrack(const void* a, const void* b){

    if(a == NULL || b == NULL){
        return false;
    }
    
    Track* first = (Track*)a;
    char* second = (char*)b;
    
    if(strcmp(first->name, second) == 0){
        return true;
    }else{
        return false;
    }
    
}
int getNumWaypoints(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->waypoints);
}

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->routes);
}

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->tracks);
}

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    List* trk = doc->tracks;
    ListIterator iter = createIterator(trk);

    int numSeg =  0;
    for(Track* getTrk = nextElement(&iter); getTrk != NULL; getTrk = nextElement(&iter)){

        numSeg += getLength(getTrk->segments);
    }

    return numSeg;
}

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc){
    
    if(doc == NULL){
        return 0;
    }
    List* wptList = doc->waypoints;
    ListIterator iter = createIterator( wptList);

    int numData = 0;
    for(Waypoint* wpt = nextElement(&iter); wpt != NULL; wpt = nextElement(&iter)){

        if(strlen(wpt->name) > 0){
            numData++;
        }
        numData += getLength(wpt->otherData);
    }

    List* rteList = doc->routes;
    ListIterator rteIter = createIterator( rteList);

    for(Route* rte = nextElement(&rteIter); rte != NULL; rte = nextElement(&rteIter)){
        
        if(strlen(rte->name) > 0){
            numData++;
        }

        numData += getLength(rte->otherData);

        wptList = rte->waypoints;
        iter = createIterator( wptList);
        for(Waypoint* wpt = nextElement(&iter); wpt != NULL; wpt = nextElement(&iter)){

            if(strlen(wpt->name) > 0){
                numData++;
            }
            numData += getLength(wpt->otherData);
        }

    }

    List* trkList = doc->tracks;
    ListIterator trkIter = createIterator( trkList);

    for(Track* trk = nextElement(&trkIter); trk!= NULL; trk = nextElement(&trkIter)){
        
        if(strlen(trk->name) > 0){
            numData++;
        }
        numData += getLength(trk->otherData);

        List* segList = trk->segments;
        ListIterator segIter = createIterator( segList);

        for(TrackSegment* seg = nextElement(&segIter); seg != NULL;seg = nextElement(&segIter)){
            
            wptList = seg->waypoints;
            iter = createIterator( wptList);
            for(Waypoint* wpt = nextElement(&iter); wpt != NULL; wpt = nextElement(&iter)){
                
                if(strlen(wpt->name) > 0){
                    numData++;
                }
                numData += getLength(wpt->otherData);
            }

        }

    }


    return numData;
}

// Function that returns a waypoint with the given name.  If more than one exists, return the first one.  
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name){

    if(doc == NULL || name == NULL){
        return NULL;
    }

    Waypoint* wpt = findElement(doc->waypoints, &findWaypoint, name);

    if(wpt !=  NULL){
        return wpt;
    }

    List* rteList = doc->routes;
    ListIterator rteIter = createIterator( rteList);

    for(Route* rte = nextElement(&rteIter); rte != NULL; rte = nextElement(&rteIter)){

        wpt = findElement(rte->waypoints, &findWaypoint, name);
        if(wpt !=  NULL){
            return wpt;
        }
    }

    List* trkList = doc->tracks;
    ListIterator trkIter = createIterator( trkList);

    for(Track* trk = nextElement(&trkIter); trk != NULL; trk = nextElement(&trkIter)){

        List* segList = trk->segments;
        ListIterator segIter = createIterator( segList);
        for(TrackSegment* seg = nextElement(&segIter); seg != NULL; seg = nextElement(&segIter)){
           
            wpt = findElement(seg->waypoints, &findWaypoint, name);
            if(wpt !=  NULL){
                return wpt;
            }
        }
    }

    return NULL;
}

// // Function that returns a track with the given name.  If more than one exists, return the first one. 
// // Return NULL if the track does not exist 
Track* getTrack(const GPXdoc* doc, char* name){
    if(doc == NULL || name == NULL){
        return NULL;
    }

    return findElement(doc->tracks, &findTrack, name);
}

// // Function that returns a route with the given name.  If more than one exists, return the first one.  
// // Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name){
    if(doc == NULL || name == NULL){
        return NULL;
    }

    return findElement(doc->routes, &findRoute, name);

}