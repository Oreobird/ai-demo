const input = $('#hiddenInput')
const chooseFile = $('#chooseLocalFile')
const camera = $('#camera')
const image = $('#image')
const hint = $('#hint')
const videoBox = $('#video-box');
const canvas = document.getElementById('canvas')
const context = canvas.getContext('2d');

const algorithm = $('#algorithm');
const formdata = new FormData();
// const host = "'+host+'";
var host = "127.0.0.1:8080";
let mediaStreamTrack;

let file = null
let Orientation = null


const uploadBtn = $('#uploadBtn')
const name = $('#name')

// 将按钮点击行为代理到hidden的input:file上。
chooseFile.click(() => input.click())


input.on('change', function() {
    uploadBtn.attr('disabled',true)
    uploadBtn.show()
    name.show()

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
        formdata.delete('name')
        formdata.delete('algorithm')
        // formdata.delete('filename')
        formdata.append('algorithm', algorithm.val());

        base64(file, Orientation, function(base64Data) {
            //base64Data:处理成功返回的图片base64
            formdata.append('filename', convertBase64UrlToBlob(base64Data));
        });

        console.log(formdata)

    });

})

name.bind('input propertychange', function() {
    //获取.input-form下的所有 <input> 元素,并实时监听用户输入
    //逻辑
    console.log(4)
    let len = name.val().trim().length
    if(len != 0) {
        uploadBtn.attr('disabled',false)
    } else {
        uploadBtn.attr('disabled',true)
    }
})

// -- ajax --
uploadBtn.click(function () {
    uploadBtn.hide()
    name.hide()

    let nameTxt = name.val()
    formdata.append('name', nameTxt);
	formdata.append('type',1);

    console.log(formdata)
    upload(formdata)
})



function upload(formdata){
    host = window.location.host;
    url = 'http://'+host+'/file/img3/upload';
    $.ajax({
        url: url,
        type: 'post',
        contentType: false,
        data: formdata,
        processData: false,
        success: function(info) {
            console.log(info)
            if(info.code != 200) {
                formatHtml(info.message)
                return;
            }
            if (info.data.result.err_code == 0) {
                if(info.data.result.pass) {
                    formatHtml('<p style="color:green">人脸注册通过！</p>')
                } else {
                    formatHtml('人脸注册不通过！')
                }
            } else if (info.data.result.err_code == -1){
                formatHtml('没有检测到人脸！')
            } else if (info.data.result.err_code == -2){
                formatHtml('检测到多张人脸！')
            } else {
                console.log('未知')
            }
            // formatHtml(info.data.result)
            
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
        result = '<h2>人脸注册</h2>'
    }
    hint.html(result).show()
}

