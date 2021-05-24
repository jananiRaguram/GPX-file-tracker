// Put all onload AJAX calls here, and event listeners

//get = get stuff from server
//post = put stuff in server
$(document).ready(function() {

    let validFiles = [];  //global variable for all valid files
    $('#dbControls').hide();
    $('#dbQueries').hide();

    let loggedIn= false;

    //display on GPX view panel
    $('#displayFile').change('click',function(e){
        //remove prev file desc table
        $('#panelTable tr').slice(1).remove();

        let fileName = $('#file').val();
        //get routes
        getRtComp(fileName);

        //get tracks
        getTrComp(fileName);

    });

    function getRtComp(fileName){
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getRt',
            data: {
                file: fileName,
            },

            success: function(data){
                console.log("filename= " + fileName);
                console.log(data);
                for(let i = 0; i<data.length; i++){
                    $('#panelTable').append('<tr>'+
                        '<td>' + "Route " + (i+1) + '<br><br>' +
                        '<button class = "rtOtherData" id =' + "rt" + (i+1) + '> additional data </button>' +
                        '</td>'+
                        '<td id ='+ "rtName" + (i+1) + ' >' + data[i].name + '</td>'+
                        '<td id ='+ "rtPts" + (i+1) + ' >' + data[i].numPoints + '</td>'+
                        '<td id ='+ "rtLen" + (i+1) + ' >' + data[i].len + '</td>'+
                        '<td id ='+ "rtLoop" + (i+1) + ' >' + data[i].loop + '</td>'+
                        '</tr>'
                    );
                }

                $('.rtOtherData').on('click', function(e){
                    let strId = this.id;
                    let onlyNum = strId.slice(2);
                    let kind = "route";
                    let obj = '{"name":"' + $('#rtName'+onlyNum).text()+ '",' + '"numPoints":' + $('#rtPts'+onlyNum).text() + ',' +'"len":' + $('#rtLen'+onlyNum).text()+".0" + ',' + '"loop":' + $('#rtLoop'+onlyNum).text() +'}';
                    getOtherData(fileName, kind, obj);
                });
            },
            fail: function(error){
                alert(error);
            }

        });
    }

    function getTrComp(fileName){
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getTr',
            data: {
                file: fileName,
            },

            success: function(data){
                console.log("filename= " + fileName);
                console.log(data);
                for(let i = 0; i<data.length; i++){
                    $('#panelTable').append('<tr>'+
                        '<td>' + "Track " + (i+1) + '<br><br>' +
                        '<button class = "trOtherData" id =' + "tr" + (i+1) + '> additional data </button>' +
                        '</td>'+
                        '<td id ='+ "trName" + (i+1) + ' >' + data[i].name + '</td>'+
                        '<td id ='+ "trPts" + (i+1) + ' >' + data[i].numPoints + '</td>'+
                        '<td id ='+ "trLen" + (i+1) + ' >' + data[i].len + '</td>'+
                        '<td id ='+ "trLoop" + (i+1) + ' >' + data[i].loop + '</td>'+
                        '</tr>'
                    );
                }


                $('.trOtherData').on('click', function(e){
                    let strId = this.id;
                    let onlyNum = strId.slice(2);
                    let kind = "track";
                    let obj = '{"name":"' + $('#trName'+onlyNum).text()+ '",' + '"numPoints":' + $('#trPts'+onlyNum).text() + ',' +'"len":' + $('#trLen'+onlyNum).text()+".0" + ',' + '"loop":' + $('#trLoop'+onlyNum).text() +'}';
                    getOtherData(fileName, kind, obj);
                });
            },
            fail: function(error){
                alert(error);
            }

        });
    }


    function getOtherData(fileName, kind, obj){
        // alert(fileName);
        console.log("obj "+  obj);
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getOther',
            data:{
                file: fileName,
                type: kind,
                str: obj,
            },

            success: function(data){
                let elem = "";
                if(data.length == 0){
                    elem = "No Other Data for Component";
                }else{
                    for(let i = 0; i<data.length; i++){
                        elem += "Name: " + data[i].name + "\n" +  "Value: " + data[i].value + "\n\n";
                    }
                }
                console.log(elem);
                alert(elem);
            },
            fail: function(error){
                console.log(error);
                alert(error);
            }
        });
    }

    //get files from server for dropdown
    $.ajax({
        type: 'get',
        dataType: 'text',
        url: '/allFiles',
        success: function(data){

            let parse = JSON.parse(data);
            let files = parse.allFiles;

            // console.log(file);
            if(files.length > 0){
                for(let i = 0; i< files.length; i++){
                    console.log("get " + files[i]);
                    validate(files[i]);
                }
            }else{

                $('#logTable').append('<tr>'+
                    '<td colspan = 6 >' + "No Files"+ '</td>'+
                    '</tr>'
                );
            }


        },
        fail: function(error){
            console.log("error");
            alert(error);
        }

    });

    function validate(fileName){
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/validate',
            data: {
                file: fileName,
                isValid: ''
            },

            success: function(data){

                if(data.isValid == "true"){
                    getFileLogRow(fileName);

                    validFiles.push(fileName);
                    //add to view panel dropdown
                    $('#file').append($('<option/>', {
                        text: fileName
                    }));

                    //add to t dropdown
                    $('#getFile').append($('<option/>', {
                        text: fileName
                    }));

                    $('#q2Files').append($('<option/>', {
                        text: fileName
                    }));

                    $('#q4Files').append($('<option/>', {
                        text: fileName
                    }));

                    $('#q5Files').append($('<option/>', {
                        text: fileName
                    }));

                    $('#q3Files').append($('<option/>', {
                        text: fileName
                    }));

                }else{

                    console.log("Following file wasn't added due to invalidity: "+fileName);

                }

            },
            fail: function(error){
                alert(error);
            }
        });

    }

    function getFileLogRow(fileName){

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getLogRow',
            data: {
                file: fileName,
                ver: '',
                creator: '',
                numWpt: '',
                numRt: '',
                numTr: ''
            },

            success: function(data){
                // console.log("filename= " + fileName);
                $('#logTable').append('<tr>'+
                    '<td>'+
                    '<a href="/uploads/' + fileName + '">' + fileName + '</a>'+
                    '</td>' +
                    '<td>' + data.ver + '</td>'+
                    '<td>' + data.creator + '</td>'+
                    '<td>' + data.numWpt + '</td>'+
                    '<td id=' + '"' + fileName + 'RtNum"' + '>' + data.numRt + '</td>'+
                    '<td>' + data.numTr + '</td>'+
                    '</tr>'
                    );

            },
            fail: function(error){
                alert(error);
            }

        });
    }

    //create .gpx file
    $('#createDoc').on('click',function(e){

        if(validInput() == true){
            let fileName = $("#createFile").val() +'.gpx';

            let str = {};
            str["version"] = parseFloat("1.1");
            str["creator"] = $('#creator').val();

            let jsonStr = JSON.stringify(str);
            // alert(jsonStr);
            e.preventDefault();
            $.ajax({
                type: 'get',
                dataType: 'text',
                url: '/createGPX',
                data: {
                    file: fileName,
                    both: jsonStr
                },
                success: function(data){

                    if(data == 1){
                        getFileLogRow(fileName);

                        validFiles.push(fileName);

                        //add to view panel dropdown
                        $('#file').append($('<option/>', {
                            text: fileName
                        }));

                        //add to addRt dropdown
                        $('#getFile').append($('<option/>', {
                            text: fileName
                        }));

                        $('#q2Files').append($('<option/>', {
                            text: fileName
                        }));

                        $('#q4Files').append($('<option/>', {
                            text: fileName
                        }));

                        $('#q5Files').append($('<option/>', {
                            text: fileName
                        }));

                        $('#q3Files').append($('<option/>', {
                            text: fileName
                        }));

                        alert("file created");
                    }else{
                        alert("Couldn't create file");
                    }

                },
                fail: function(error){
                    console.log(error);
                }
            });
        }

    });

    function validInput(){

        let fileName = $("#createFile").val();
        if(fileName.length == 0){
            alert("Must include file name");
            return false;
        }

        let fileWithGPX = fileName + '.gpx';

        for(let i = 0; i < validFiles.length; i++){
            if(fileWithGPX == validFiles[i]){
                alert("file with name " + fileName + " already created");
                return false;
            }
        }

        let creator = $("#creator").val();

        if(creator.length == 0){
            alert("Include creator name");
            return false;
        }

        return true;
    }

    //add rt
    $('#numWpts').change('click', function(e){
        for(let i = 1; i < 5; i++){
            let val = "#wpt" + i;
            $(val).hide();
        }

        let num = parseInt($("#numWpts").val());
        for(let i = 1; i < num +1; i++){
            let val = "#wpt" + i;
            $(val).show();
        }
    });

    $('.addTo').on('click',function(e){

        let nameProv = true;
        let fileName = $('#getFile').val();
        if(fileName == null){
            alert("Must Choose a File");
            nameProv = false;
        }
        //validate lat and lon componenets

        let num = parseInt($("#numWpts").val());
        let jsonpWptStr = "";
        let jsonRtStr = "";
        let itsGood = 0;

        for(let i = 1; i< num+1; i++){
            let latVal = "#lat" + i;
            let lonVal = "#lon" + i;
            let inLat = $(latVal).val();

            let inLon = $(lonVal).val();

            //check if empty
            if(inLat.length > 0 && inLon.length > 0 ){

                //check if in range
                if(inLat >= -90.0 && inLat <= 90.0 && inLon >= -180.0 && inLon < 180.0){

                    let wptStr = {};
                    wptStr["lat"] = parseFloat( $(latVal).val());
                    wptStr["lon"] = parseFloat( $(lonVal).val());

                    if(i < num){
                        jsonpWptStr += JSON.stringify(wptStr) + "|";
                    }else{
                        jsonpWptStr += JSON.stringify(wptStr);
                    }

                    let rtStr = {};
                    let name = $('#rtName').val();

                    rtStr["name"] = name;
                    jsonRtStr = JSON.stringify(rtStr);

                    itsGood++;

                }else{
                    alert("latitude range: -90.0 to 90.0 \n" + "longitude range: -180.0 to 179.99");
                }
            }else{
                alert("Must include latitude and longitude values");
            }
        }
        console.log("whole thing " + jsonpWptStr);

        if(loggedIn == false){
            alert("log into DataBase");
        }else{

            if(itsGood == num && nameProv == true){
                $.ajax({
                    type: 'get',
                    dataType: 'text',
                    url: '/addRt',
                    data:{
                        file: fileName,
                        wptStr: jsonpWptStr,
                        rtStr: jsonRtStr,
                        numWpt: num
                    },
                    success: function(data){

                        if(data == 0){
                            alert("Couldn't add route");
                        }else{
                            console.log("Added route");

                            let fileRt = fileName+"RtNum";

                            let numRtCell = document.getElementById(fileRt).textContent;
                            let numRts = parseInt(numRtCell) + 1;

                            //write added number of routes back to page
                            document.getElementById(fileRt).innerHTML = numRts;

                            //updates
                            // $('#dbUpdates').on('click', function(e){
                                $.ajax({
                                    type: 'get',
                                    dataType: 'json',
                                    url: '/update',
                                    data:{
                                        files: validFiles
                                    },
                                    success: function(data){
                                        console.log(data);
                                        getdbStatus();

                                    },
                                    fail: function(error){
                                        console.log(error);
                                    }
                                });

                                // e.preventDefault();

                            // });
                        }


                        document.getElementById("addRt").reset();

                    },
                    fail: function(error){
                        console.log(error);
                    }
                });
            }
        }

        e.preventDefault();

        for(let i = 1; i < 5; i++){
            let val = "#wpt" + i;
            $(val).hide();
        }
    });

    //find pathBtwn
    $('.findPath').on('click', function(e){
        $('#pathsBtwnTable tr').slice(1).remove();


        let startLat = $('#startLat').val();
        let startLon = $('#startLon').val();

        let endLat = $('#endLat').val();
        let endLon = $('#endLon').val();

        let delta = $('#delta').val();
        if(startLat.length > 0 && startLon.length > 0 && endLat.length > 0 && endLon.length > 0 && delta.length > 0){
            if(startLat >= -90.0 && startLat <= 90.0 && endLat >= -90.0 && endLat <= 90.0 && endLon >= -180.0 && endLon < 180.0 && startLon >= -180.0 && startLon < 180.0){


                let startLatF = parseFloat(startLat);
                let startLonF = parseFloat(startLon);

                let endLatF = parseFloat(endLat);
                let endLonF = parseFloat(endLon);

                let deltaI = parseFloat(delta);

                for(let i = 0 ; i<validFiles.length; i++){
                    getRtPath(validFiles[i], startLatF, startLonF, endLatF, endLonF, deltaI);
                    getTrkPath(validFiles[i], startLatF, startLonF, endLatF, endLonF, deltaI);
                }
                // alert("in");
                document.getElementById("pathBtwn").reset();
            }else{
                alert("latitude range: -90.0 to 90.0 \n" + "longitude range: -180.0 to 179.99");
            }

        }else{
            alert("All fields must be filled");
        }
        e.preventDefault();


    });

    function getRtPath(fileName, startLatF, startLonF, endLatF, endLonF, deltaI){
        console.log(fileName + " btwn data rt ");
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getRtBtwnPath',
            data:{
                file: fileName,
                sLat: startLatF,
                sLon: startLonF,
                eLat: endLatF,
                eLon: endLonF,
                dlt: deltaI
            },
            success: function(data){
                for(let i = 0; i<data.length; i++){
                    $('#pathsBtwnTable').append('<tr>'+
                        '<td>' + fileName + '<br>' + "Route " + (i+1) + '<br><br>' +
                        '<button class = "rtOtherData" id=' + "rtBtwn"+(i+1)+'> additional data </button>' +
                        '</td>'+
                        '<td id=' + "rtBtwnName"+(i+1)+'>' + data[i].name + '</td>'+
                        '<td id='+"rtBtwnPts"+(i+1)+'>' + data[i].numPoints + '</td>'+
                        '<td id='+"rtBtwnLen"+(i+1)+'>' + data[i].len + '</td>'+
                        '<td id=' + "rtBtwnLoop"+(i+1)+'>' + data[i].loop + '</td>'+
                        '</tr>'
                    );
                }

                $('.rtOtherData').on('click', function(e){
                    let strId = this.id;
                    let onlyNum = strId.slice(6);
                    let kind = "route";
                    let obj = '{"name":"' + $('#rtBtwnName'+onlyNum).text()+ '",' + '"numPoints":' + $('#rtBtwnPts'+onlyNum).text() + ',' +'"len":' + $('#rtBtwnLen'+onlyNum).text()+".0" + ',' + '"loop":' + $('#rtBtwnLoop'+onlyNum).text() +'}';
                    getOtherData(fileName, kind, obj);
                });

            },
            fail: function(error){
                console.log(error);
            }

        });
    }

    function getTrkPath(fileName, startLatF, startLonF, endLatF, endLonF, deltaI){
        console.log(fileName + " btwn data trk ");

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getTrkBtwnPath',
            data:{
                file: fileName,
                sLat: startLatF,
                sLon: startLonF,
                eLat: endLatF,
                eLon: endLonF,
                dlt: deltaI
            },
            success: function(data){
                console.log(data);

                for(let i = 0; i<data.length; i++){
                    $('#pathsBtwnTable').append('<tr>'+
                    '<td>' + fileName + '<br>' + "Track " + (i+1) + '<br><br>' +
                    '<button class = "trOtherData" id=' + "trBtwn"+(i+1)+'> additional data </button>' +
                    '</td>'+
                    '<td id=' + "trBtwnName"+(i+1)+'>' + data[i].name + '</td>'+
                    '<td id='+"trBtwnPts"+(i+1)+'>' + data[i].numPoints + '</td>'+
                    '<td id='+"trBtwnLen"+(i+1)+'>' + data[i].len + '</td>'+
                    '<td id=' + "trBtwnLoop"+(i+1)+'>' + data[i].loop + '</td>'+
                    '</tr>'
                );
                }
                $('.trOtherData').on('click', function(e){
                    let strId = this.id;
                    let onlyNum = strId.slice(6);
                    let kind = "track";
                    let obj = '{"name":"' + $('#trBtwnName'+onlyNum).text()+ '",' + '"numPoints":' + $('#trBtwnPts'+onlyNum).text() + ',' +'"len":' + $('#trBtwnLen'+onlyNum).text()+".0" + ',' + '"loop":' + $('#trBtwnLoop'+onlyNum).text() +'}';
                    getOtherData(fileName, kind, obj);
                });
            },
            fail: function(error){
                console.log(error);
            }

        });
    }

    $('.renameVal').on('click', function(e){

        if(loggedIn == false){
            alert("log into DataBase");
            itsGood = false;
        }

        let type= "";
        let itsGood = true;
        let fileName = $("#file").val();
        if(fileName == null){
            alert("Must Choose a File");
            itsGood = false;
        }

        if(document.getElementById("trk").checked == true){
            type = "trk";
        }else if(document.getElementById("rt").checked == true){
            type = "rt";
        }else{
            alert("Must check box for route or track");
            itsGood = false;
        }

        let typeNum = parseInt($('#typeNum').val());
        if(typeNum.length == 0){
            alert("Must include the component number from the table");
            itsGood = false;

        }

        if(type == "trk"){
            if(document.getElementById("trName"+typeNum) == null){
                alert("track with number "+ typeNum + " does not exist");
                itsGood = false;
            }
        }else if(type == "rt"){
            if(document.getElementById("rtName"+typeNum) == null){
                alert("route with number "+ typeNum + " does not exist");
                itsGood = false;
            }
        }


        let newName = $('#newName').val();
        if(newName.length == 0){
            alert("Must include new component name");
            itsGood = false;

        }

        if(itsGood == true){
            let fileName = $("#file").val();

            $.ajax({
                type: 'get',
                dataType: 'text',
                url: '/renameCmp',
                data:{
                    file: fileName,
                    newName: newName,
                    typeNum: typeNum,
                    type: type
                },
                success: function(data){
                    if(data == 0){
                        alert("failed to rename route/track for " + fileName);
                    }else{
                        console.log("rename successful");
                        $('#panelTable tr').slice(1).remove();

                        //get routes
                        getRtComp(fileName);

                        //get tracks
                        getTrComp(fileName);
                    }

                    //update database
                    // $('#dbUpdates').on('click', function(e){
                        $.ajax({
                            type: 'get',
                            dataType: 'json',
                            url: '/update',
                            data:{
                                files: validFiles
                            },
                            success: function(data){
                                console.log(data);
                                getdbStatus();

                            },
                            fail: function(error){
                                console.log(error);
                            }
                        });

                        // e.preventDefault();

                    // });
                },
                fail: function(error){
                    alert(error);
                }
            });

            document.getElementById("rename").reset();
            document.getElementById("displayFile").reset();
        }
        e.preventDefault();

        // location.reload();
    });


    //A4
    /****************************************************************************/

    //login
    $('.login').on('click', function(e) {
        let uN = $('#username').val();
        let uP = $('#password').val();
        let dbName = $('#dbName').val();
        let host = 'dursley.socs.uoguelph.ca';

        $.ajax({

            type: 'get',
            dataType: 'json',
            url: '/dbLogin',
            data:{
                    uN: uN,
                    uP: uP,
                    dbName: dbName,
                    host: host
            },
            success: function (data){
                if(data == false){
                    alert("Couldn't login. Please try again");
                }else{
                    console.log("connected to database");
                    $('#dbControls').show();
                    $('#showRts').hide();
                    $('#showQ2').hide();
                    $('#showQ3').hide();
                    $('#showQ4').hide();
                    $('#showQ5').hide();

                    if(validFiles.length != 0){
                        $('#dbQueries').show();
                    }else{
                        $("#dbControls").append($("<h3>").text("Add GPX files for queries"));
                    }

                    alert("Logged in");
                    loggedIn = true;
                }
            },
            fail: function (error){
                alert(error);
            }
        });

        e.preventDefault();
    });

    /****************************************************************************/
    //store all Files
    $('#dbStoreFiles').on('click', function(e){
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/storeFiles',
            data:{
                files: validFiles
            },
            success: function(data){
                console.log(data);
                getdbStatus();

            },
            fail: function(error){
                console.log(error);
            }
        });

        e.preventDefault();

    });



    //clear
    $('#dbClear').on('click', function(e){
        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/dbClear',
            success: function(data){
                console.log(data);
                getdbStatus();

            },
            fail: function(error){
                console.log(error);
            }
        });
        e.preventDefault();


    });


    $('#dbStatus').on('click', function(e){

        getdbStatus();
    });

    function getdbStatus(){
        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/dbStatus',
            success: function(data){

                if(data.result == true){
                    let status = "Database has " + data.numFiles + " files, " + data.numRts + " routes, and " + data.numPts + " points";
                    alert(status);
                }else{
                    alert("Couldn't get database status, may need to login again");
                }
            },
            fail: function(data){
                console.log(error);
            }

        });
    }

    /////////////////////////////////////////////////////////////////////

    //queries


    //q1 = display all routes sorted by name or Length (let user)
    $('#allRts').on('click', function(e){
        $('#rtsTable tr').slice(1).remove();

        let sortVal= $('#rtSort').val();
        if(sortVal == null){
            alert("choose sort method");
        }else{
            $.ajax({
                type: 'get',
                dataType:'JSON',
                url: '/allRts',
                data:{
                    sortVal: sortVal
                },
                success: function(data){

                    if(data.result == true){
                        let rtListParse = JSON.parse(data.rtsList);

                        if(rtListParse.length == 0){
                            $('#rtsTable').append('<tr>'+
                                '<td colspan = 6 >' + "No Routes"+ '</td>'+
                                '</tr>'
                            );
                        }else{
                            for(let i = 0; i < rtListParse.length; i++){

                                $('#rtsTable').append('<tr>'+
                                  '<td>' + rtListParse[i].name + '</td>'+
                                  '<td>' + rtListParse[i].len + '</td>'+
                                  '</tr>'
                                );
                            }
                        }

                        $('#showRts').show();

                    }else{
                        alert("Couldn't get database status, may need to login again");
                    }
                },
                fail: function(error){
                    console.log(error);
                }
            });

        }

    });

    //q2 = display routes from a file; shows names and lengths of all routes and file names, let user sort by name/len
    $('#rtsFromFile').on('click', function(e){
        $('#q2Table tr').slice(1).remove();

        let file = $('#q2Files').val();
        let sortVal = $('#q2Sort').val();
        if(sortVal == null || file == null){
            alert("choose sort method and file");
        }else{
            $.ajax({
                type: 'get',
                dataType:'JSON',
                url: '/fileRts',
                data:{
                    sortVal: sortVal,
                    file: file
                },
                success: function(data){

                    if(data.result == true){
                        let rtListParse = JSON.parse(data.rtsList);
                        $('#q2RtName').text("File Name: " + file);

                        if(rtListParse.length == 0){
                            $('#q2Table').append('<tr>'+
                                '<td colspan = 6 >' + "No Routes"+ '</td>'+
                                '</tr>'
                            );
                        }else{
                            for(let i = 0; i < rtListParse.length; i++){
                                $('#q2Table').append('<tr>'+
                                  '<td>' + rtListParse[i].name + '</td>'+
                                  '<td>' + rtListParse[i].len + '</td>'+
                                  '</tr>'
                                );
                            }
                        }
                        $('#showQ2').show();

                    }else{
                        alert("Couldn't get database status, may need to login again");
                    }
                },
                fail: function(error){
                    console.log(error);
                }
            });

        }
    });


    //q4 = all points from specific file; let user order by rt name/len (if no name can order whatever; "unnamed route 1..."), for points order by ptIdx,
    $('#q4Sub').on('click', function(e){
        $('#q4Table tr').slice(1).remove();

        let file = $('#q4Files').val();
        let sortVal = $('#q4Sort').val();
        if(sortVal == null || file == null){
            alert("choose sort method and file");
        }else{
            console.log(sortVal);
            $.ajax({
                type: 'get',
                dataType:'JSON',
                url: '/q4',
                data:{
                    sortVal: sortVal,
                    file: file
                },
                success: function(data){

                    if(data.result == true){
                        let rtListParse = JSON.parse(data.rtsList);
                        if(rtListParse.length == 0){
                            $('#q4Table').append('<tr>'+
                                '<td colspan = 6 >' + "No Routes"+ '</td>'+
                                '</tr>'
                            );
                        }else{
                            for(let i = 0; i < rtListParse.length; i++){
                                $('#q4Table').append('<tr>'+
                                  '<td>' + rtListParse[i].rtName + '</td>'+
                                  '<td>' + rtListParse[i].rtLen + '</td>'+
                                  '<td>' + rtListParse[i].ptIdx + '</td>'+
                                  '<td>' + rtListParse[i].ptName + '</td>'+
                                  '<td>' + rtListParse[i].lat + '</td>'+
                                  '<td>' + rtListParse[i].lon + '</td>'+
                                  '</tr>'
                                );
                            }
                        }
                        $('#showQ4').show();

                    }else{
                        alert("Couldn't get database status, may need to login again");
                    }
                },
                fail: function(error){
                    console.log(error);
                }
            });

        }
    });

    //q5 = N shortest/longest routes from specific file; user picks file, N, shortest/longest. show names and lengths of all routes and file name (let user choose sort)
    $('#q5Sub').on('click', function(e){
        $('#q5Table tr').slice(1).remove();

        let file = $('#q5Files').val();
        let sortVal = $('#q5Sort').val();
        let numRows = $('#q5Rows').val();
        let sortVal2 = $('#q5Sort2').val();
        if(sortVal == null || file == null || numRows.length == 0 || sortVal2 == null){
            alert("choose sort method,file, sort method and number of routes");
        }else{
            console.log(sortVal);
            $.ajax({
                type: 'get',
                dataType:'JSON',
                url: '/q5',
                data:{
                    sortVal: sortVal,
                    file: file,
                    numRows: numRows,
                    sortVal2: sortVal2
                },
                success: function(data){

                    if(data.result == true){
                        let rtListParse = JSON.parse(data.rtsList);
                        $('#q5RtName').text("File Name: " + file);

                        if(rtListParse.length == 0){
                            $('#q5Table').append('<tr>'+
                                '<td colspan = 6 >' + "No Routes"+ '</td>'+
                                '</tr>'
                            );
                        }else{
                            for(let i = 0; i < rtListParse.length; i++){
                                // let parse = JSON.parse(rtListParse[i]);
                                $('#q5Table').append('<tr>'+
                                  '<td>' + rtListParse[i].rtName + '</td>'+
                                  '<td>' + rtListParse[i].rtLen + '</td>'+
                                  '</tr>'
                                );
                            }
                        }
                        $('#showQ5').show();

                    }else{
                        alert("Couldn't get database status, may need to login again");
                    }
                },
                fail: function(error){
                    console.log(error);
                }
            });

        }
    });


    //q3 = show all points of specific route, ordered by point index
    $('#q3DisplayFile').change('click',function(e){
        //remove prev file desc
        $('#q3RtNum').empty();

        let fileName = $('#q3Files').val();
        //get routes
        q3GetRtComp(fileName);

    });


    function q3GetRtComp(fileName){
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getRt',
            data: {
                file: fileName,
            },

            success: function(data){
                console.log("filename= " + fileName);
                console.log(data);
                for(let i = 0; i<data.length; i++){

                    $('#q3RtNum').append($('<option/>', {
                        text: i+1 + ". " + data[i].name
                    }));
                }
            },
            fail: function(error){
                alert(error);
            }

        });
    }

    $('#q3Sub').on('click', function(e){
        $('#q3Table tr').slice(1).remove();

        let fileName = $('#q3Files').val();
        let rtNum = $('#q3RtNum').val().substring(0,1);

        console.log(fileName);
        console.log(rtNum);
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/q3',
            data: {
                file: fileName,
                rtNum: rtNum,
            },

            success: function(data){
                if(data.result == true){
                    let rtListParse = JSON.parse(data.rtsList);
                    $('#q3RtName').text("Route Name: " + data.rtName);

                    for(let i = 0; i < rtListParse.length; i++){
                        $('#q3Table').append('<tr>'+
                          '<td>' + rtListParse[i].ptIdx + '</td>'+
                          '<td>' + rtListParse[i].ptName + '</td>'+
                          '<td>' + rtListParse[i].lat + '</td>'+
                          '<td>' + rtListParse[i].lon + '</td>'+
                          '</tr>'
                        );
                    }
                    $('#showQ3').show();

                }else{
                    alert("Couldn't get database status, may need to login again");
                }
            },
            fail: function(error){
                alert(error);
            }

        });
    });



});
