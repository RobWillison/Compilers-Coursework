var gulp = require('gulp'),
    watch = require('gulp-watch'),
    notify = require("gulp-notify"),
    exec = require('gulp-exec');

gulp.task('make', function (done) {
    exec('make all', function (err, stdout, stderr) {

        if (err) {
          notify().write("BUILD FAILD!!!!! :(");
        } else {
          gulp.start('test');
        }
      });

    done();
});

gulp.task('test', function (done) {
    exec('python3 test.py', function (err, stdout, stderr) {

        if (err) {
          notify().write(stdout);
        } else {
          notify().write("TEST SUCCESS");
        }
      });

    done();
});


gulp.task('watch', function () {
    // Endless stream mode
    return watch('*.c', function () {
        gulp.start('make');
    });
});
