const input = $('#hiddenInput')
const chooseFile = $('#chooseLocalFile')

const image = $('#image')
const hint = $('#hint')
const videoBox = $('#video-box')
const canvas = document.getElementById('canvas')
const context = canvas.getContext('2d')

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

let c2Width = 0
let c2Hedth = 0

// 将按钮点击行为代理到hidden的input:file上。
chooseFile.click(() => input.click())

/*algorithm.on('change',function(){
    if(formdata.get('filename')){
        formdata.set('algorithm',$(this).val())
        upload(formdata)
    }
})*/
input.on('change', function() {

    mediaStreamTrack && mediaStreamTrack.stop();
    videoBox.hide()
    // image.hide()
    // debugger
    file = this.files[0]
    updatePicture(URL.createObjectURL(file))
    // console.log(file)


    //获取照片方向角属性，用户旋转控制
    EXIF.getData(file, function() {
        // console.log(EXIF.pretty(this));
        EXIF.getAllTags(this);
        Orientation = EXIF.getTag(this, 'Orientation');

        // 上传
        // var formdata = new FormData()
        formdata.delete('img_data')
        formdata.delete('line_points')
        formdata.delete('algorithm')
        formdata.delete('filename')
        formdata.append('algorithm', algorithm.val());

        base64(file, Orientation, function(base64Data) {
            //base64Data:处理成功返回的图片base64
            formdata.append('img_data', convertBase64UrlToBlob(base64Data));
        });

        console.log(formdata)

    });

})

// -- ajax --
confirm.click(function () {
    cross.hide()
    confirm.hide()

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
    // formdata.append('line_points', JSON.stringify(data));
    formdata.append('line_points', jstr);

    //[{x1,y1},{x2,y2}]

    console.log('----------------')
    // console.log(JSON.stringify(formdata.getAll('line_points')))
    console.log(formdata)
    upload(formdata)

})

function upload(formdata) {
    host = window.location.host;
    Url = 'http://'+host+'/file/img2/upload';
    $.ajax({
        url: Url,
        type: 'post',
        contentType: false,
        data: formdata,
        processData: false,
        headers: {
            //"Content-Type": "application/json;charset=utf-8"
        },
        success: function(info) {

            console.log(info)
            if(info.err_code == 0 && info.alarm) {
                console.log('触发警报')
            } else if(info.err_code == 0 && !info.alarm) {
                console.log('无警报')
            }

            ctx.clearRect(0, 0, c2Width, c2Hedth)
            updatePicture('data:image/jpeg;base64,'+info.data.result.img_data)
            image.show()
            canvas.style.display = 'none'
            canvas2.style.display = 'none'

        },
        error: function(err) {
            console.log(err)
        }
    });
}

/*function upload(formdata) {
    host = window.location.host;
    Url = 'http://'+host+'/file/img/upload';
    $.ajax({
        url: ' http://10.101.70.234/file/img2/upload',
        type: 'post',
        contentType: false,
        data: formdata,
        processData: false,
        headers: {
            "Content-Type": "application/json;charset=utf-8"
        },
        success: function(info) {

            console.log(info)
            if(info.err_code == 0 && info.alarm) {
                console.log('触发警报')
            } else if(info.err_code == 0 && !info.alarm) {
                console.log('无警报')
            }

            updatePicture('data:image/jpeg;base64,'+info.data.result.img_data)
            image.show()
            ctx.clearRect(0, 0, c2Width, c2Hedth)
            canvas.style.display = 'none'
            canvas2.style.display = 'none'

        },
        error: function(err) {
            console.log(err)
        }
    });
}*/

//
function base64(file, Orientation, backData) {
    // console.log(Orientation)
    var reader = new FileReader();
    var image = new Image();
    reader.onload = function() { // 文件加载完处理
        var result = this.result;
        image.onload = function() { // 图片加载完处理
            var imgScale = imgScaleW(1000, this.width, this.height);
            if (Orientation != '') {
                switch (Orientation) {
                    case 6:
                        canvas.width = imgScale.height;
                        canvas.height = imgScale.width;
                        context.rotate(90 * Math.PI / 180);
                        context.drawImage(image, 0, -imgScale.height, imgScale.width, imgScale.height);
                        break;
                    case 8:
                        canvas.width = imgScale.height;
                        canvas.height = imgScale.width;
                        context.rotate(270 * Math.PI / 180);
                        context.drawImage(image, -imgScale.width, 0, imgScale.width, imgScale.height);
                        break;
                    case 3:
                        canvas.width = imgScale.width;
                        canvas.height = imgScale.height;
                        //需要180度旋转
                        context.rotate(180 * Math.PI / 180);
                        context.drawImage(image, -imgScale.width, -imgScale.height, imgScale.width, imgScale.height);
                        break;
                    default:
                        canvas.width = imgScale.width;
                        canvas.height = imgScale.height;
                        context.drawImage(image, 0, 0, imgScale.width, imgScale.height);
                        break;
                }
            } else {
                canvas.width = imgScale.width;
                canvas.height = imgScale.height;
                context.drawImage(image, 0, 0, imgScale.width, imgScale.height);
            }

            var dataURL = canvas.toDataURL('image/jpeg'); // 图片base64
            backData(dataURL); //dataURL:处理成功返回的图片base64
            // console.log(dataURL)

            /*------- 分界线划分 -----*/
            document.getElementById('image').style.display = 'none'
            canvas.style.display = 'block'
            canvas2.style.display = 'block'

            ifHidden = true
            cross.text('隐藏分界线').show()

            canvas2.width = canvas.width
            canvas2.height = canvas.height
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

                confirm.show()
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
        // console.log(result);
        image.src = result

    };
    reader.readAsDataURL(file);
}

function imgScaleW(maxWidth, width, height) {
    /* maxWidth:宽度或者高度最大值
     * width：宽度
     * height：高度
     * */
    var imgScale = {};
    var w = 0;
    var h = 0;
    if (width <= maxWidth && height <= maxWidth) { // 如果图片宽高都小于限制的最大值,不用缩放
        imgScale = {
            width: width,
            height: height
        }
    } else {
        if (width >= height) { // 如果图片宽大于高
            w = maxWidth;
            h = Math.ceil(maxWidth * height / width);
        } else { // 如果图片高大于宽
            h = maxWidth;
            w = Math.ceil(maxWidth * width / height);
        }
        imgScale = {
            width: w,
            height: h
        }
    }
    return imgScale;
}

/**
 * 将以base64的图片url数据转换为Blob
 * @param urlData
 * 用url方式表示的base64图片数据
 */
function convertBase64UrlToBlob(urlData) {
    var arr = urlData.split(','),
        mime = arr[0].match(/:(.*?);/)[1],
        bstr = atob(arr[1]),
        n = bstr.length,
        u8arr = new Uint8Array(n);
    while (n--) {
        u8arr[n] = bstr.charCodeAt(n);
    }
    return new Blob([u8arr], {
        type: mime
    });
}

function updatePicture(picUrl){
    image.attr('src', picUrl);
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

/* 右侧返回信息 */
function formatHtml(r){
    let result = ''
    if(typeof r === 'string'){
        result = r
    }else{
        result = ''
    }
    hint.html(result).show()
}
