package top.kikt.ffmpeg_sdl_example

import android.net.Uri
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
            val tmpFile = File(filesDir, "tmp.jpg")
            try {
                FFmpeg.getFirstImage(
//                    "http://media.w3.org/2010/05/sintel/trailer.mp4",
                    "http://clips.vorwaerts-gmbh.de/big_buck_bunny.mp4",
//                    "http://vjs.zencdn.net/v/oceans.mp4",
                    tmpFile.absolutePath
                )
            } catch (e: Exception) {
                Log.i("MainActivity", "发生错误", e)
            }

            Log.i("MainActivity", tmpFile.length().toString())
            iv_result.setImageURI(Uri.fromFile(tmpFile))
        }


    }
}
