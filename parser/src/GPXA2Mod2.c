#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "GPXParser.h"
#include "GPXHelper.h"
#include "LinkedListAPI.h"
#include <string.h>
#include <stdlib.h>


float round10(float len){

    //rounded up
    float a = ceil(len / 10) * 10;

    //rounded down
    float b = floor(len / 10) * 10;

    return (fabs(len - a) > fabs(len - b))? b : a;
}

/********************************************************/
float getRouteLen(const Route *rt){
    if(rt == NULL){
        return 0;
    }

    //in meters
    float sum = 0;
    ListIterator iterWpt = createIterator(rt->waypoints);

    for(Waypoint* getWpt = nextElement(&iterWpt); getWpt != NULL; getWpt= nextElement(&iterWpt)){

        ListIterator iterNext = iterWpt;
        Waypoint* nextWpt = nextElement(&iterNext);
        if(nextWpt != NULL){

          sum += pointDistance(getWpt->latitude, getWpt->longitude, nextWpt->latitude,nextWpt->longitude );

        }
    }

    return sum;
}

/********************************************************/
float getTrackLen(const Track *tr){

    if(tr == NULL){
        return 0;
    }

    ListIterator iterSeg = createIterator(tr->segments);
    //in meters
    float sum = 0;

    double prevSegWptLat = 0;
    double prevSegWptLon = 0;

    int flag = 0;

    for(TrackSegment* seg = nextElement(&iterSeg); seg!= NULL; seg = nextElement(&iterSeg)){

        ListIterator iterWpt = createIterator(seg->waypoints);

        for(Waypoint* getWpt = nextElement(&iterWpt); getWpt != NULL; getWpt= nextElement(&iterWpt)){

            //distance between last point in prev seg and first point in current seg
            //only when switch between segments
            if(flag == 1){

                sum += pointDistance(prevSegWptLat, prevSegWptLon, getWpt->latitude, getWpt->longitude );

            }

            ListIterator iterNext = iterWpt;
            Waypoint* nextWpt = nextElement(&iterNext);
            if(nextWpt != NULL){
                sum += pointDistance(getWpt->latitude, getWpt->longitude, nextWpt->latitude,nextWpt->longitude );

                prevSegWptLat = nextWpt->latitude;
                prevSegWptLon = nextWpt->longitude;
                flag = 0;
            }
        }

        flag = 1;

    }

    return sum;
}

/********************************************************/
int numRoutesWithLength(const GPXdoc* doc, float len, float delta){

    if(doc == NULL || len < 0 || delta < 0){
        return 0;
    }

    int count = 0;

    ListIterator iterRte = createIterator(doc->routes);

    for(Route* rte = nextElement(&iterRte); rte != NULL; rte = nextElement(&iterRte)){

        float rteLen = getRouteLen(rte);

        if(rteLen == len){
            count++;
        }else if(fabs(len - rteLen) <= delta){
            //if (len - other routeLen) <= delta; they are equal
            count++;
        }
    }

    return count;
}


/********************************************************/
int numTracksWithLength(const GPXdoc* doc, float len, float delta){

    if(doc == NULL || len < 0 || delta < 0){
        return 0;
    }

    int count = 0;

    ListIterator iterTrk = createIterator(doc->tracks);

    for(Track* trk = nextElement(&iterTrk); trk != NULL; trk = nextElement(&iterTrk)){

        float trkLen = getTrackLen(trk);

        if(trkLen == len){
            count++;
        }else if(fabs(len - trkLen) <= delta){
            //if (len - other trkLen) <= delta; they are equal
            count++;
        }
    }

    return count;
}


/********************************************************/
bool isLoopRoute(const Route* route, float delta){

    if(route == NULL){
        return false;
    }else if(delta < 0){
        return false;
    }

    ListIterator iterWpt = createIterator(route->waypoints);
    int len = getLength(route->waypoints);
    if(len < 4){
      return false;
    }
    int i = 0;
    double firstLat;
    double firstLon;
    double lastLat;
    double lastLon;

    for(Waypoint* wpt = nextElement(&iterWpt); wpt != NULL; wpt = nextElement(&iterWpt)){

      if(i == 0){
        firstLat = wpt->latitude;
        firstLon = wpt->longitude;
      }else if(i == len -1){
        lastLat = wpt->latitude;
        lastLon = wpt->longitude;
      }
      i++;
    }

    double lenBtwnWpt = pointDistance(firstLat, firstLon, lastLat, lastLon);

    /**route to form loop must:
        - have at least 4 wpt
        - have distance of less than delta btwn first and last points
    */
    if(lenBtwnWpt <= delta){
        return true;
    }

    return false;
}


/********************************************************/
bool isLoopTrack(const Track *tr, float delta){
     if(tr == NULL || delta < 0){
        return false;
    }

    int len = getLength(tr->segments);
    if(len == 0){
      return false;
    }

    int numWpt = 0;
    ListIterator iterSeg = createIterator(tr->segments);
    for(TrackSegment* trkSeg = nextElement(&iterSeg); trkSeg != NULL; trkSeg = nextElement(&iterSeg)){

      ListIterator iterWpt = createIterator(trkSeg->waypoints);
      for(Waypoint* wpt = nextElement(&iterWpt); wpt != NULL; wpt = nextElement(&iterWpt)){
        numWpt++;
      }
    }

    if(numWpt < 4){
      return false;
    }

    TrackSegment* seg1 = getFromFront(tr->segments);
    Waypoint* wpt1 = getFromFront(seg1->waypoints);

    TrackSegment* seg2 = getFromBack(tr->segments);
    Waypoint* wpt2 = getFromBack(seg2->waypoints);

    double firstLat = wpt1->latitude;
    double firstLon = wpt1->longitude;
    double lastLat = wpt2->latitude;
    double lastLon = wpt2->longitude;


    double lenBtwnWpt = pointDistance(firstLat, firstLon, lastLat, lastLon);

    /*route to form loop must:
        - have at least 4 wpt in total in one or more segments
        - have distance of less than delta btwn first and last points
    */
    if(lenBtwnWpt <= delta && numWpt >= 4){
        return true;
    }

    return false;
}


/********************************************************/
List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){

  if(doc == NULL){
    return NULL;
  }

  List* rteList = initializeList(&routeToString, &deleteList, &compareRoutes);
  ListIterator iterRte = createIterator(doc->routes);

  for(Route* rte = nextElement(&iterRte); rte != NULL; rte = nextElement(&iterRte)){

    Waypoint* wpt1 = getFromFront(rte->waypoints);
    Waypoint* wpt2 = getFromBack(rte->waypoints);

    double firstLat = wpt1->latitude;
    double firstLon = wpt1->longitude;
    double lastLat = wpt2->latitude;
    double lastLon = wpt2->longitude;

    double d1 = pointDistance(sourceLat, sourceLong, firstLat, firstLon);
    double d2 = pointDistance(lastLat, lastLon, destLat, destLong);

    if(d1 <= delta && d2 <= delta){
      insertBack(rteList, rte );
    }
  }

  if(getLength(rteList) == 0){
      freeList(rteList);
      return NULL;
  }


  return rteList;
}


/********************************************************/
void deleteList(void* data){
  return;
}

/********************************************************/
List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    if(doc == NULL){
      return NULL;
    }

    List* trkList = initializeList(&trackToString, &deleteList, &compareTracks);
    ListIterator iterTrk = createIterator(doc->tracks);

    for(Track* tr = nextElement(&iterTrk); tr!= NULL; tr = nextElement(&iterTrk)){

        TrackSegment* seg1 = getFromFront(tr->segments);
        Waypoint* wpt1 = getFromFront(seg1->waypoints);

        TrackSegment* seg2 = getFromBack(tr->segments);
        Waypoint* wpt2 = getFromBack(seg2->waypoints);

        double firstLat = wpt1->latitude;
        double firstLon = wpt1->longitude;
        double lastLat = wpt2->latitude;
        double lastLon = wpt2->longitude;

        double d1 = pointDistance(sourceLat, sourceLong, firstLat, firstLon);
        double d2 = pointDistance(lastLat, lastLon, destLat, destLong);

        if(d1 <= delta && d2 <= delta){
            insertBack(trkList, tr );
        }
    }

    if(getLength(trkList) == 0){
        freeList(trkList);
        return NULL;
    }

    return trkList;
}


/********************************************************/
double pointDistance(double srcLat, double srcLon, double destLat, double destLon){
  const int R = 6371*pow(10,3); //in meters

  const float lat1 = srcLat * (M_PI/180);

  const double lat2 = destLat * (M_PI/180);

  const double deltaLat = (destLat - srcLat) * M_PI/180;
  const double deltaLon = (destLon - srcLon) * M_PI/180;

  const double a = sin(deltaLat/2) * sin(deltaLat/2) + cos(lat1) * cos(lat2) * sin(deltaLon/2) * sin(deltaLon/2);
  const double c = 2 * atan2(sqrt(a), sqrt(1-a));
  const double d = R * c;

  return d;
}
