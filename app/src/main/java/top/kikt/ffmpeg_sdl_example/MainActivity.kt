package top.kikt.ffmpeg_sdl_example

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        sample_text.setOnClickListener {
            FFmpeg.dumpInfo("http://media.w3.org/2010/05/sintel/trailer.mp4")
        }

    }
}
