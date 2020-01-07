package top.kikt.ffmpeg_sdl_example

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)


        sample_button.setOnClickListener {
            FFmpeg.dumpInfo("http://media.w3.org/2010/05/sintel/trailer.mp4")
            val tmpFile = File(filesDir, "tmp.jpg")
            try {
                FFmpeg.getFirstImage(
                    "http://media.w3.org/2010/05/sintel/trailer.mp4",
                    tmpFile.absolutePath
                )
            } catch (e: Exception) {
                Log.i("MainActivity", "发生错误", e)
            }

            Log.i("MainActivity", tmpFile.length().toString())

        }

    }
}
