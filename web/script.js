const input = $('#hiddenInput')
const chooseFile = $('#chooseLocalFile')
const camera = $('#camera')
const image = $('#image')
const hint = $('#hint')
const videoBox = $('#video-box');
const canvas = document.getElementById('canvas')
const context = canvas.getContext('2d');
const capture = $('#capture')
const video = $('#video')[0]
const algorithm = $('#algorithm');
const formdata = new FormData();
// const host = "'+host+'";
var host = "127.0.0.1";
let mediaStreamTrack;

let file = null
let Orientation = null

// 将按钮点击行为代理到hidden的input:file上。
chooseFile.click(() => input.click())

algorithm.on('change',function(){
    if(formdata.get('filename')){
        formdata.set('algorithm',$(this).val())
        upload(formdata)
    }
})

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
        formdata.delete('algorithm')
        formdata.delete('filename')
        formdata.append('algorithm', algorithm.val());

        if (Orientation != 1) {
            base64(file, Orientation, function(base64Data) {
                //base64Data:处理成功返回的图片base64
                // $('.backimg').attr("src",base64Data);
                formdata.append('filename', convertBase64UrlToBlob(base64Data));
                upload(formdata);

                // console.log('11'+formdata);
            });
        } else {
            if (file.size > 1024 * 500) {
                base64(file, Orientation, function(base64Data) {
                    //base64Data:处理成功返回的图片base64
                    // $('.backimg').attr("src",base64Data);
                    formdata.append('filename', convertBase64UrlToBlob(base64Data));
                    upload(formdata);
                    // console.log('22'+formdata);
                });
            } else {
                formdata.append('filename', file);
                upload(formdata);
                // console.log(formdata);
            }
        }



    });


})

camera.click(() => {
    navigator.mediaDevices.getUserMedia({
        // 和video dom元素一致
        video: {width:480,height:320}
    }).then(stream => {
        console.log(stream)
        //将视频流设置为video元素的源
        let URL = window.URL || window.webkitURL
        video.src = URL.createObjectURL(stream);
        //播放视频
        video.play();
        //canvas.style.display = 'none'
        videoBox.show()
        image.hide()
        mediaStreamTrack = stream.getTracks()[0]
    }, err => {
        alert("你的浏览器不支持访问用户媒体设备")
        console.error(err)
    })
})

//capture.addEventListener('click', function() {
capture.click(() => {
    // console.log(canvas.width);
    context.clearRect(0,0,canvas.width,canvas.height);
    canvas.width = '480'
    canvas.height = '320'
    //绘制画面
    context.drawImage(video, 0, 0, 480, 320);
    mediaStreamTrack && mediaStreamTrack.stop();
    let canvasImg = convertBase64UrlToBlob(canvas.toDataURL('image/png'));
    updatePicture(canvas.toDataURL('image/png'));
    formdata.delete('algorithm')
    formdata.delete('filename')
    // formdata = new FormData()
    formdata.append('filename', canvasImg);
    formdata.append('algorithm', algorithm.val());
    // console.log(formdata)
    upload(formdata);
    videoBox.hide()
    //canvas.style.display = 'block'
});

function upload(formdata){
    host = window.location.host;
    url = 'http://'+host+'/file/img/upload';
    $.ajax({
        url: url,
        type: 'post',
        contentType: false,
        data: formdata,
        processData: false,
        success: function(info) {
            if(info.code != 200) {
                formatHtml(info.message)
                return;
            }
            if (info.data.result.err_code == -1) {
                formatHtml('没有检测到人脸！')
                image.show()
                return;
            } else if (info.data.result.err_code == -2){
                formatHtml('检测到多张人脸！')
                // image.show()
                // return;
            } else {
                console.log('111111111')
                // image.show()
                //canvas.style.display = 'none'
            }

            updatePicture('data:image/jpeg;base64,'+info.data.result.img_data)
            image.show()
            formatHtml(info.data.result)
            
        },
        error: function(err) {
            console.log(err)
        }
    });

}

//
function base64(file, Orientation, backData) {
    // console.log(Orientation)
    var reader = new FileReader();
    var image = new Image();
    reader.onload = function() { // 文件加载完处理
        var result = this.result;
        image.onload = function() { // 图片加载完处理
            var imgScale = imgScaleW(1000, this.width, this.height);
            canvas.width = imgScale.width;
            canvas.height = imgScale.height;
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

            //context.drawImage(image, 0, 0, imgScale.width, imgScale.height);
            var dataURL = canvas.toDataURL('image/jpeg'); // 图片base64
            context.clearRect(0, 0, imgScale.width, imgScale.height); // 清除画布
            backData(dataURL); //dataURL:处理成功返回的图片base64
            console.log(dataURL)
        }
        // console.log(result);
        image.src = result;

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
 *            用url方式表示的base64图片数据
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

function formatHtml(r){
    let result = ''
    if(typeof r === 'string'){
        result = r
    }else{
        result = `
            <h2>图片质量</h2>         
            <p><span>偏转角度：</span>${r.quality.yaw.pass ? '<em class="green">'+r.quality.yaw.value+'</em>':'<em class="red">' +r.quality.yaw.value+'</em>'}<span class="fright">[0-${r.quality.yaw.threshold}]</span></p>
            <p><span>俯仰角度：</span>${r.quality.pitch.pass ? '<em class="green">'+r.quality.pitch.value+'</em>':'<em class="red">' +r.quality.pitch.value+'</em>'}<span class="fright">[0-${r.quality.pitch.threshold}]</span></p>
            <p><span>滚转角度：</span>${r.quality.roll.pass ? '<em class="green">'+r.quality.roll.value+'</em>':'<em class="red">' +r.quality.roll.value+'</em>'}<span class="fright">[0-${r.quality.roll.threshold}]</span></p>
            <p><span>清晰度：</span>${r.quality.clarity.pass ? '<em class="green">'+r.quality.clarity.value+'</em>':'<em class="red">' +r.quality.clarity.value+'</em>'}<span class="fright">[${r.quality.clarity.threshold}-1]</span></p>
            <p><span>对比度：</span>${r.quality.contrast.pass ? '<em class="green">'+r.quality.contrast.value+'</em>':'<em class="red">' +r.quality.contrast.value+'</em>'}<span class="fright">[${r.quality.contrast.threshold}-1]</span></p>
            <p><span>位置：</span>${r.quality.pos.pass ? '<em class="green">'+r.quality.pos.value+'</em>':'<em class="red">' +r.quality.pos.value+'</em>'}<span class="fright">[${r.quality.pos.threshold}-1]</span></p>
            <p><span>大小：</span>${r.quality.size.pass ? '<em class="green">'+r.quality.size.value+'</em>':'<em class="red">' +r.quality.size.value+'</em>'}<span class="fright">[${r.quality.size.threshold}-1]</span></p>
            <p><span>明亮度：</span>${r.quality.brightness.pass ? '<em class="green">'+r.quality.brightness.value+'</em>':'<em class="red">' +r.quality.brightness.value+'</em>'}<span class="fright">[${r.quality.brightness.threshold}-1]</span></p>
			<p><span>表情：</span>${r.quality.emotion.pass ? '<em class="green">'+r.quality.emotion.value+'</em>':'<em class="red">' +r.quality.emotion.value+'</em>'}<span class="fright">[${r.quality.emotion.threshold}-1]</span></p>
            <p><span>遮挡：</span>${r.quality.occlusion.pass ? '<em class="green">'+r.quality.occlusion.value+'</em>':'<em class="red">' +r.quality.occlusion.value+'</em>'}<span class="fright">[${r.quality.occlusion.threshold}-1]</span></p>
			<p class="fb"><span>总分：</span>${r.quality.total.pass ? '<em class="green">'+r.quality.total.value+'</em>':'<em class="red">' +r.quality.total.value+'</em>'}<span class="fright">[${r.quality.total.threshold}-100]</span></p>
            <p class="result">
                <span>结论：</span>
                ${r.pass ?'<strong class="green">图片合格</strong>' : '<strong class="red">图片不合格</strong>'}
            </p>
        `
    }
    hint.html(result).show()
}

