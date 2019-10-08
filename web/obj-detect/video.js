const input = $('#hiddenInput')
const chooseFile = $('#chooseLocalFile')
const load = $('#loading')

const image = $('#image')
const hint = $('#hint')
const videoBox = $('#video')
const progress = $('#progress')

const algorithm = $('#algorithm')
const formdata = new FormData()

let host = "127.0.0.1"
let Url
let mediaStreamTrack
let file = null
let Orientation = null

const cross = $('#cross')
const confirm = $('#confirm')
const canvas2 = document.querySelector('#canvas2')
const ctx = canvas2.getContext('2d')
const dv = $('.view-box').eq(0)
let x1, y1, x2, y2
let arr
let arrX1, arrY1, arrX2, arrY2
let ifHidden = true
let loading = false

let c2Width = 0
let c2Hedth = 0

// 将按钮点击行为代理到hidden的input:file上。
chooseFile.click(() => input.click())

input.on('change', function() {

    formdata.delete('video_data')
    formdata.delete('line_points')

    file = this.files[0]
    var videoSize = file.size;
	if (loading) {
		alert("视频正在处理中，请等待结束后重试");
	} else if (videoSize >= 100 * 1024 * 1024) {
        alert("视频过大,请重新拍摄，上传视频不能大于100M");
    } else {
        var path = URL.createObjectURL(file)
        // path = '/video/output_20190222145334.mp4'
        console.log(path)
		
        var html = '<video width="850" height="480" controls="controls" src="' + path + '" video_id="" width=100% height=100% poster="" >' +	//video标签去掉
            '<source src="' + path + '" type="video/ogg">' +
            '<source src="' + path + '" type="video/mp4">' +
            '<source src="' + path + '" type="video/webm">' +
            '<object data="' + path + '" >' +
            '<embed  src="' + path + '">' +
            '</object>' +
            '</video>';

        videoBox.html(html)

		
        formdata.append('video_data', file);

        ifHidden = true		
		cross.text('隐藏分界线').show()
        videoBox.show()
        canvas2.style.display = 'block'
        image.hide()

        draw()

        // var reader = new FileReader();
        // reader.readAsDataURL(file);//调用自带方法进行转换
        // reader.onload = function (e) {
        //     console.log(this.result)
        //     var img = this.result;
        //     var imgNum = img.split(";base64,");
        //     var imgBase = imgNum[1];
        //     console.log(img)
        //     formdata.append('video_data', file);
        //     //formdata.append('video_data', img);
        //     console.log(file)
        //
        //     canvas2.style.display = 'block'
        //     ifHidden = true
        //     cross.text('隐藏分界线').show()
        //     draw()
        // }
    }


})

function draw() {

    ifHidden = true
    cross.text('隐藏分界线').show()


    canvas2.width = '850'
    canvas2.height = '480'
    c2Width = canvas2.width
    c2Hedth = canvas2.height

    dv.bind('mousedown',function(e){
        ctx.clearRect(0, 0, c2Width, c2Hedth)
        x1 = 0
        y1 = 0
        x2 = 0
        y2 = 0
        x1 = e.offsetX
        y1 = e.offsetY
    })

    dv.bind('mouseup',function(e){
        x2 = e.offsetX
        y2 = e.offsetY
        drawline(x1, y1, x2, y2)

        /* 坐标转化为百分比 */
        /* 第1个点的坐标为（x1,y1) */
        /* 第2个点的坐标为（x2,y2) */
        arrX1 = x1 / canvas2.width
        arrY1 = y1 / canvas2.height
        arrX2 = x2 / canvas2.width
        arrY2 = y2 / canvas2.height
        // console.log(arrX1, arrY1, arrX2, arrY2)

		if (!loading)
		{
			confirm.show()
		}
		
    })

    /* 画线 */
    function drawline(a1, a2, b1, b2) {
        ctx.beginPath()
        ctx.moveTo(a1, a2)
        ctx.lineTo(b1, b2)
        ctx.strokeStyle = 'red'
        ctx.stroke()
        ctx.closePath()

    }
}

// -- ajax --
confirm.click(function () {
    cross.hide()
    confirm.hide()
    load.show()
    progress.text('0%').show()
	
    var data=[]
    data[0]={
        'x': arrX1.toFixed(4),
        'y':arrY1.toFixed(4)
    }
    data[1]={
        'x': arrX2.toFixed(4),
        'y':arrY2.toFixed(4)
    }

    console.log('1:'+JSON.stringify(data));
    var jstr = '[{"x":'+arrX1.toFixed(4)+',"y":'+arrY1.toFixed(4)+'},{"x":'+arrX2.toFixed(4)+',"y":'+arrY2.toFixed(4)+'}]';
    console.log('2:'+jstr);

    formdata.append('line_points', jstr);


    console.log('----------------')

    console.log(formdata)
    upload(formdata)

})


function upload(formdata) {
    host = window.location.host;
    Url = 'http://'+host+':8080/file/video/progress';
    console.log(Url)
	loading = true

    $.ajax({
        url: 'http://'+host+':8080/file/video/upload',
        type: 'POST',
        contentType: false,
        data: formdata,
        processData: false,
        success: function(info) {

            console.log('1111')
            console.log(info)
            if(info.code == 200) {
                var result = info.data.result
                console.log(result)
                var videoSource = result.video_source
                console.log(videoSource)
				
                var timer = setInterval(function () {

                $.ajax({
                    url: 'http://'+host+':8080/file/video/progress?video_source='+videoSource,
                    type: 'GET',
                    success: function(data) {

                        console.log('11112222')
                        console.log(data)
                        if(data.code == 200) {
                            var resultV = data.data.result
                            var pro = resultV.progress
                            console.log(resultV.progress)
                            progress.text(pro)
                            if (resultV.video_path) {
                                progress.hide()
                                clearInterval(timer);
                                var path = resultV.video_path
                                console.log(path)
                                var html = '<video width="850" height="480" controls="controls" src="' + path + '" video_id="" width=100% height=100% poster="" >' +	//video标签去掉
                                    '<source src="' + path + '" type="video/ogg">' +
                                    '<source src="' + path + '" type="video/mp4">' +
                                    '<source src="' + path + '" type="video/webm">' +
                                    '<object data="' + path + '" >' +
                                    '<embed  src="' + path + '">' +
                                    '</object>' +
                                    '</video>';
                                videoBox.html(html)
                                load.hide()
								loading = false
                            }

                        } else {
                            console.log('分析失败')
                            clearInterval(timer);
                            load.hide()
                        }

                        ctx.clearRect(0, 0, c2Width, c2Hedth)
                        canvas2.style.display = 'none'

                    },
                    error: function(err) {
                        console.log('222')
                        clearInterval(timer);
                        console.log(err)
                    }
                });


                },2000);

            } else {
                console.log('分析失败')
                load.hide()
            }

            ctx.clearRect(0, 0, c2Width, c2Hedth)
            canvas2.style.display = 'none'

        },
        error: function(err) {
            console.log('222')
            console.log(err)
        }
    });



}






/* 隐藏/显示分界线 */
cross.click(() => {
    if(ifHidden){
        canvas2.style.display = 'none'
        cross.text('显示分界线')
        ifHidden = false
    } else{
        canvas2.style.display = 'block'
        cross.text('隐藏分界线')
    ifHidden=true
    }
})


