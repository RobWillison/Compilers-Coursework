var gulp = require('gulp'),
    watch = require('gulp-watch'),
    notify = require("gulp-notify"),
    exec = require('gulp-exec');


gulp.task('test', function (done) {
    console.log(exec('python3 test.py', function (err, stdout, stderr) {
        console.log(stdout);
        if (err) {
          notify().write(stdout);
        } else {
          notify().write("SUCCESS");
        }
      }));

    done();
});


gulp.task('watch', function () {
    // Endless stream mode
    return watch('*.c', function () {
        gulp.start('test');
    });
});
