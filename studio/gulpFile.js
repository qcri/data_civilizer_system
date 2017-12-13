// Dependencies
var gulp = require('gulp');
var nodemon = require('gulp-nodemon');
var notify = require('gulp-notify');
var livereload = require('gulp-livereload');
var gulpSSH = require('gulp-ssh');

var serverSSHConfig = {
    host: '192.168.1.75',
    port: 22,
    username: 'badr',
    password: 'ootNehyt2'
}

var gulpServerSSH = new gulpSSH({
    ignoreErrors: false,
    sshConfig: serverSSHConfig
});

var serverDeploymentCommands = [
    'cd /home/badr/rheem_project/Rheem',
    'git pull origin staging',
    'npm install',
    'pm2 restart all'
];

gulp.task('start', function() {
    // configure nodemon
    nodemon({
        script: 'server.js',
        watch: ["public", "public/*", "models", "routes", "controllers", "helpers", "server.js"],
        ext: 'js'
    }).on('restart', function(err){
        gulp.src('server.js')
        .pipe(notify('Restarting app'));
  });
});

gulp.task('deploy', function() {
    return gulpStagingSSH
        .exec(stagingDeploymentCommands, {filePath: 'staging-shell.log'})
        .pipe(gulp.dest('logs'))
});
